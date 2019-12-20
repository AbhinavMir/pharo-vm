/* aio.c -- asynchronous file i/o
 * 
 *   Copyright (C) 1996-2006 by Ian Piumarta and other authors/contributors
 *                              listed elsewhere in this file.
 *   All rights reserved.
 *   
 *   This file is part of Unix Squeak.
 * 
 *   Permission is hereby granted, free of charge, to any person obtaining a
 *   copy of this software and associated documentation files (the "Software"),
 *   to deal in the Software without restriction, including without limitation
 *   the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *   and/or sell copies of the Software, and to permit persons to whom the
 *   Software is furnished to do so, subject to the following conditions:
 * 
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 * 
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *   DEALINGS IN THE SOFTWARE.
 */

/* Authors: Ian.Piumarta@squeakland.org, eliot.miranda@gmail.com
 * 
 * Last edited: Tue Mar 29 13:06:00 PDT 2016
 */

#include "sqaio.h"
#include "pharovm/debug.h"
#include "pharovm/semaphores/platformSemaphore.h"

#ifdef HAVE_CONFIG_H

# include "config.h"

# ifdef HAVE_UNISTD_H
#   include <sys/types.h>
#   include <unistd.h>
# endif /* HAVE_UNISTD_H */

# ifdef NEED_GETHOSTNAME_P
    extern int gethostname();
# endif

# include <stdio.h>
# include <signal.h>
# include <errno.h>
# include <fcntl.h>
# include <sys/ioctl.h>

# ifdef HAVE_SYS_TIME_H
#   include <sys/time.h>
# else
#   include <time.h>
# endif

# if HAVE_KQUEUE
#   include <sys/event.h>
# elif HAVE_EPOLL
#   include <sys/epoll.h>
# elif HAVE_SELECT
#   include <sys/select.h>
# endif

# ifndef FIONBIO
#   ifdef HAVE_SYS_FILIO_H
#     include <sys/filio.h>
#   endif
#   ifndef FIONBIO
#     ifdef FIOSNBIO
#       define FIONBIO FIOSNBIO
#     else
#       error: FIONBIO is not defined
#     endif
#   endif
# endif

# if __sun__
  # include <sys/sockio.h>
  # define signal(a, b) sigset(a, b)
# endif

#else /* !HAVE_CONFIG_H -- assume lowest common demoninator */

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <errno.h>
# include <signal.h>
# include <sys/types.h>
# include <sys/time.h>
# include <sys/select.h>
# include <sys/ioctl.h>
# include <fcntl.h>

#endif /* !HAVE_CONFIG_H */

/* function to inform the VM about idle time */
extern void addIdleUsecs(long idleUsecs);

#if defined(AIO_DEBUG)
long	aioLastTick = 0;
long	aioThisTick = 0;

#endif

#define _DO_FLAG_TYPE()	do { _DO(AIO_R, rd) _DO(AIO_W, wr) _DO(AIO_X, ex) } while (0)

static aioHandler rdHandler[FD_SETSIZE];
static aioHandler wrHandler[FD_SETSIZE];
static aioHandler exHandler[FD_SETSIZE];

static void *clientData[FD_SETSIZE];

static int maxFd;
static fd_set fdMask;		/* handled by aio	 */
static fd_set rdMask;		/* handle read		 */
static fd_set wrMask;		/* handle write		 */
static fd_set exMask;		/* handle exception	 */
static fd_set xdMask;		/* external descriptor	 */


void heartbeat_poll_enter(long microSeconds);
void heartbeat_poll_exit(long microSeconds);

Semaphore* interruptFIFOMutex;
int aio_in_sleep = 0;
int aio_request_interrupt = 0;

static void 
undefinedHandler(int fd, void *clientData, int flags)
{
	fprintf(stderr, "undefined handler called (fd %d, flags %x)\n", fd, flags);
}

#ifdef AIO_DEBUG
const char *
__shortFileName(const char *full__FILE__name)
{
	const char *p = strrchr(full__FILE__name, '/');

	return p ? p + 1 : full__FILE__name;
}
static char *
handlerName(aioHandler h)
{
	if (h == undefinedHandler)
		return "undefinedHandler";
#ifdef DEBUG_SOCKETS
	{
		extern char *socketHandlerName(aioHandler);

		return socketHandlerName(h);
	}
#endif
	return "***unknown***";
}

#endif

/* initialise asynchronous i/o */

static int signal_pipe_fd[2];

void 
aioInit(void)
{
	extern void forceInterruptCheck(int);	/* not really, but hey */

	interruptFIFOMutex = platform_semaphore_new(1);

	FD_ZERO(&fdMask);
	FD_ZERO(&rdMask);
	FD_ZERO(&wrMask);
	FD_ZERO(&exMask);
	FD_ZERO(&xdMask);
	maxFd = 0;

	if (pipe(signal_pipe_fd) == -1) {
	    perror("pipe");
	    exit(-1);
	}

	//signal(SIGPIPE, SIG_IGN);
	signal(SIGIO, forceInterruptCheck);
}


/* disable handlers and close all handled non-exteral descriptors */

void 
aioFini(void)
{
	int	fd;

	for (fd = 0; fd < maxFd; fd++)
		if (FD_ISSET(fd, &fdMask) && !(FD_ISSET(fd, &xdMask))) {
			aioDisable(fd);
			close(fd);
			FD_CLR(fd, &fdMask);
			FD_CLR(fd, &rdMask);
			FD_CLR(fd, &wrMask);
			FD_CLR(fd, &exMask);
		}
	while (maxFd && !FD_ISSET(maxFd - 1, &fdMask))
		--maxFd;
	signal(SIGPIPE, SIG_DFL);
}


/*
 * answer whether i/o becomes possible within the given number of
 * microSeconds
 */
#define max(x,y) (((x)>(y))?(x):(y))

long	pollpip = 0;		/* set in sqUnixMain.c by -pollpip arg */

#if COGMTVM
/*
 * If on the MT VM and pollpip > 1 only pip if a threaded FFI call is in
 * progress, which we infer from disownCount being non-zero.
 */
extern long disownCount;

# define SHOULD_TICK() (pollpip == 1 || (pollpip > 1 && disownCount))
#else
# define SHOULD_TICK() pollpip
#endif

static char *ticks = "-\\|/";
static char *ticker = "";
static int tickCount = 0;

#define TICKS_PER_CHAR 10
#define DO_TICK(bool)				\
do if ((bool) && !(++tickCount % TICKS_PER_CHAR)) {		\
	fprintf(stderr, "\r%c\r", *ticker);		\
	if (!*ticker++) ticker= ticks;			\
} while (0)

/*
 * I Try to clear all the data available in the pipe, so it does not passes the limit of data.
 * Do not call me outside the mutex area of interruptFIFOMutex.
 */
void
aio_flush_pipe(int fd){
	char buf[1024];
    int bytesRead;
    int selectResult;
	fd_set readFD;
	struct timeval tv;

	tv.tv_sec = 0;
	tv.tv_usec = 0;


	FD_SET(fd, &readFD);

	do {
		selectResult = select(fd + 1, &readFD, NULL, NULL, &tv);

		if(FD_ISSET(fd, &readFD)){
			bytesRead = read(fd, &buf, 1024);
		}else{
			bytesRead = 0;
		}
	} while(bytesRead == 1024);
}

/**
 * I check the status of the flags signalling an interruption.
 * If there is a pending interruption I return 1, and clear the pipe.
 * The aioPoll will not execute.
 * If there is not, I return 0. The AIOpoll has to run.
 */
int
aio_checkAndEnterSleep(int fd){
	interruptFIFOMutex->wait(interruptFIFOMutex);

	if(aio_request_interrupt){
		aio_request_interrupt = false;
		aio_in_sleep = false;
		aio_flush_pipe(fd);

		interruptFIFOMutex->signal(interruptFIFOMutex);
		return 1;
	}

	aio_in_sleep = true;

	interruptFIFOMutex->signal(interruptFIFOMutex);
    return 0;
}

/**
 * I flush the pipe and mark the exit of the aioPoll
 */
void
aio_flushAndExitSleep(int fd){
	interruptFIFOMutex->wait(interruptFIFOMutex);

	aio_flush_pipe(fd);

	aio_request_interrupt = false;
	aio_in_sleep = false;

	interruptFIFOMutex->signal(interruptFIFOMutex);

}

long 
aioPoll(long microSeconds)
{
    
	int	fd;
	fd_set	rd, wr, ex;
	unsigned long long us;
	int maxFdToUse;
	long remainingMicroSeconds;

	DO_TICK(SHOULD_TICK());

	/*
	 * get out early if there is no pending i/o and no need to relinquish
	 * cpu
	 */

#ifdef TARGET_OS_IS_IPHONE
	if (maxFd == 0)
		return 0;
#else
	if ((maxFd == 0) && (microSeconds == 0))
		return 0;
#endif

    if(aio_checkAndEnterSleep(signal_pipe_fd[0])){
    	return 1;
    }
    
	rd = rdMask;
	wr = wrMask;
	ex = exMask;
	us = ioUTCMicroseconds();

	remainingMicroSeconds = microSeconds;

	FD_SET(signal_pipe_fd[0], &rd);

	maxFdToUse = maxFd > (signal_pipe_fd[0] + 1) ? maxFd : signal_pipe_fd[0] + 1;

	heartbeat_poll_enter(microSeconds);

	for (;;) {
		struct timeval tv;
		int	n;
		unsigned long long now;

		tv.tv_sec = remainingMicroSeconds / 1000000;
		tv.tv_usec = remainingMicroSeconds % 1000000;
		n = select(maxFdToUse, &rd, &wr, &ex, &tv);
		if (n > 0)
			break;
		if (n == 0) {
			if (remainingMicroSeconds)
				addIdleUsecs(remainingMicroSeconds);
			heartbeat_poll_exit(microSeconds);
	    	logTrace("n == 0");
			return 0;
		}
		if (errno && (EINTR != errno)) {
			fprintf(stderr, "errno %d\n", errno);
			perror("select");
			heartbeat_poll_exit(microSeconds);
	    	logTrace("error");
			return 0;
		}
		now = ioUTCMicroseconds();
		remainingMicroSeconds -= max(now - us, 1);

		if (remainingMicroSeconds <= 0){
			heartbeat_poll_exit(microSeconds);
	    	logTrace("remainingMicroSeconds <= 0");
			return 0;
		}
		us = now;
	}

	heartbeat_poll_exit(microSeconds);
	aio_flushAndExitSleep(signal_pipe_fd[0]);

	for (fd = 0; fd < maxFd; ++fd) {
#undef _DO
#define _DO(FLAG, TYPE)								\
		if (FD_ISSET(fd, &TYPE)) {					\
			aioHandler handler= TYPE##Handler[fd];	\
			FD_CLR(fd, &TYPE##Mask);				\
			TYPE##Handler[fd]= undefinedHandler;	\
			handler(fd, clientData[fd], FLAG);		\
		}
		_DO_FLAG_TYPE();
	}

	logTrace("processed");
	return 1;
}

/*
 * This function is used to interrupt a aioPoll.
 * Used when signalling a Pharo semaphore to re-wake the VM and execute code of the image.
 */

void
aioInterruptPoll(){
	int n;

	interruptFIFOMutex->wait(interruptFIFOMutex);

	if(aio_in_sleep){
		n = write(signal_pipe_fd[1], "1", 1);

		if(n != 1){
			perror("write");
		}
	}

	aio_request_interrupt = true;

	interruptFIFOMutex->signal(interruptFIFOMutex);
}


/*
 * sleep for microSeconds or until i/o becomes possible, avoiding sleeping in
 * select() if timeout too small
 */

long 
aioSleepForUsecs(long microSeconds)
{
	/* This makes no sense at all.  This simply increases latency.  It calls
	 * aioPoll and then immediately enters a nonasleep for the requested time.
	 * Hence if there is pending i/o it will prevent responding to that i/o for
	 * the requested sleep.  Not a good idea. eem May 2017.
	 */
#if defined(HAVE_NANOSLEEP) && 0
	if (microSeconds < (1000000 / 60)) {	/* < 1 timeslice? */
		if (!aioPoll(0)) {
			struct timespec rqtp = {0, microSeconds * 1000};
			struct timespec rmtp = {0, 0};

			nanosleep(&rqtp, &rmtp);
			addIdleUsecs((rqtp.tv_nsec - rmtp.tv_nsec) / 1000);
			microSeconds = 0;	/* poll but don't block */
		}
	}
#endif
	/* This makes perfect sense.  Poll with a timeout of microSeconds, returning
	 * when the timeout has elapsed or i/o is possible, whichever is sooner.
	 */
	return aioPoll(microSeconds);
}


/* enable asynchronous notification for a descriptor */

void 
aioEnable(int fd, void *data, int flags)
{
	FPRINTF((stderr, "aioEnable(%d)\n", fd));
	if (fd < 0) {
		FPRINTF((stderr, "aioEnable(%d): IGNORED\n", fd));
		return;
	}
	if (FD_ISSET(fd, &fdMask)) {
		fprintf(stderr, "aioEnable: descriptor %d already enabled\n", fd);
		return;
	}
	clientData[fd] = data;
	rdHandler[fd] = wrHandler[fd] = exHandler[fd] = undefinedHandler;
	FD_SET(fd, &fdMask);
	FD_CLR(fd, &rdMask);
	FD_CLR(fd, &wrMask);
	FD_CLR(fd, &exMask);
	if (fd >= maxFd)
		maxFd = fd + 1;
	if (flags & AIO_EXT) {
		FD_SET(fd, &xdMask);
		/* we should not set NBIO ourselves on external descriptors! */
	}
	else {
		/*
		 * enable non-blocking asynchronous i/o and delivery of SIGIO
		 * to the active process
		 */
		int	arg;

		FD_CLR(fd, &xdMask);

#if defined(O_ASYNC)
		if (fcntl(fd, F_SETOWN, getpid()) < 0)
			perror("fcntl(F_SETOWN, getpid())");
		if ((arg = fcntl(fd, F_GETFL, 0)) < 0)
			perror("fcntl(F_GETFL)");
		if (fcntl(fd, F_SETFL, arg | O_NONBLOCK | O_ASYNC) < 0)
			perror("fcntl(F_SETFL, O_ASYNC)");

#elif defined(FASYNC)
		if (fcntl(fd, F_SETOWN, getpid()) < 0)
			perror("fcntl(F_SETOWN, getpid())");
		if ((arg = fcntl(fd, F_GETFL, 0)) < 0)
			perror("fcntl(F_GETFL)");
		if (fcntl(fd, F_SETFL, arg | O_NONBLOCK | FASYNC) < 0)
			perror("fcntl(F_SETFL, FASYNC)");

#elif defined(FIOASYNC)
		arg = getpid();
		if (ioctl(fd, SIOCSPGRP, &arg) < 0)
			perror("ioctl(SIOCSPGRP, getpid())");
		arg = 1;
		if (ioctl(fd, FIOASYNC, &arg) < 0)
			perror("ioctl(FIOASYNC, 1)");
#endif
	}
}


/* install/change the handler for a descriptor */

void 
aioHandle(int fd, aioHandler handlerFn, int mask)
{
	FPRINTF((stderr, "aioHandle(%d, %s, %d)\n", fd, handlerName(handlerFn), mask));
	if (fd < 0) {
		FPRINTF((stderr, "aioHandle(%d): IGNORED\n", fd));
		return;
	}
#undef _DO
#define _DO(FLAG, TYPE)					\
    if (mask & FLAG) {					\
      FD_SET(fd, &TYPE##Mask);			\
      TYPE##Handler[fd]= handlerFn;		\
    }
	_DO_FLAG_TYPE();
}


/* temporarily suspend asynchronous notification for a descriptor */

void 
aioSuspend(int fd, int mask)
{
	if (fd < 0) {
		FPRINTF((stderr, "aioSuspend(%d): IGNORED\n", fd));
		return;
	}
	FPRINTF((stderr, "aioSuspend(%d)\n", fd));
#undef _DO
#define _DO(FLAG, TYPE)							\
	if (mask & FLAG) {							\
		FD_CLR(fd, &TYPE##Mask);				\
		TYPE##Handler[fd]= undefinedHandler;	\
	}
	_DO_FLAG_TYPE();
}


/* definitively disable asynchronous notification for a descriptor */

void 
aioDisable(int fd)
{
	if (fd < 0) {
		FPRINTF((stderr, "aioDisable(%d): IGNORED\n", fd));
		return;
	}
	FPRINTF((stderr, "aioDisable(%d)\n", fd));
	aioSuspend(fd, AIO_RWX);
	FD_CLR(fd, &xdMask);
	FD_CLR(fd, &fdMask);
	rdHandler[fd] = wrHandler[fd] = exHandler[fd] = 0;
	clientData[fd] = 0;
	/* keep maxFd accurate (drops to zero if no more sockets) */
	while (maxFd && !FD_ISSET(maxFd - 1, &fdMask))
		--maxFd;
}
