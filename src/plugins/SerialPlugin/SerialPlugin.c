/* Automatically generated by
	SmartSyntaxPluginCodeGenerator VMMaker.oscog-eem.261 uuid: eeb310a3-23e0-41f6-8a92-5749b798e623
   from
	SerialPlugin VMMaker.oscog-eem.261 uuid: eeb310a3-23e0-41f6-8a92-5749b798e623
 */
static char __buildInfo[] = "SerialPlugin VMMaker.oscog-eem.261 uuid: eeb310a3-23e0-41f6-8a92-5749b798e623 " __DATE__ ;



#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Default EXPORT macro that does nothing (see comment in sq.h): */
#define EXPORT(returnType) returnType

/* Do not include the entire sq.h file but just those parts needed. */
/*  The virtual machine proxy definition */
#include "sqVirtualMachine.h"
/* Configuration options */
#include "sqConfig.h"
/* Platform specific definitions */
#include "sqPlatformSpecific.h"

#define true 1
#define false 0
#define null 0  /* using 'null' because nil is predefined in Think C */
#ifdef SQUEAK_BUILTIN_PLUGIN
#undef EXPORT
// was #undef EXPORT(returnType) but screws NorCroft cc
#define EXPORT(returnType) static returnType
#endif

#include "SerialPlugin.h"
#include "sqMemoryAccess.h"


/*** Constants ***/
#define BytesPerWord 4


/*** Function Prototypes ***/
static VirtualMachine * getInterpreter(void);
EXPORT(const char*) getModuleName(void);
static sqInt halt(void);
EXPORT(sqInt) initialiseModule(void);
static sqInt msg(char *s);
EXPORT(sqInt) primitiveSerialPortClose(void);
EXPORT(sqInt) primitiveSerialPortOpen(void);
EXPORT(sqInt) primitiveSerialPortOpenByName(void);
EXPORT(sqInt) primitiveSerialPortRead(void);
EXPORT(sqInt) primitiveSerialPortReadByName(void);
EXPORT(sqInt) primitiveSerialPortWrite(void);
EXPORT(sqInt) primitiveSerialPortWriteByName(void);
EXPORT(sqInt) setInterpreter(struct VirtualMachine*anInterpreter);
EXPORT(sqInt) shutdownModule(void);
static void sqAssert(sqInt aBool);


/*** Variables ***/

#if !defined(SQUEAK_BUILTIN_PLUGIN)
static sqInt (*byteSizeOf)(sqInt oop);
static sqInt (*failed)(void);
static void * (*firstIndexableField)(sqInt oop);
static sqInt (*integerObjectOf)(sqInt value);
static sqInt (*isBytes)(sqInt oop);
static sqInt (*pop)(sqInt nItems);
static sqInt (*popthenPush)(sqInt nItems, sqInt oop);
static sqInt (*slotSizeOf)(sqInt oop);
static sqInt (*stackIntegerValue)(sqInt offset);
static sqInt (*stackValue)(sqInt offset);
static sqInt (*success)(sqInt aBoolean);
#else /* !defined(SQUEAK_BUILTIN_PLUGIN) */
extern sqInt byteSizeOf(sqInt oop);
extern sqInt failed(void);
extern void * firstIndexableField(sqInt oop);
extern sqInt integerObjectOf(sqInt value);
extern sqInt isBytes(sqInt oop);
extern sqInt pop(sqInt nItems);
extern sqInt popthenPush(sqInt nItems, sqInt oop);
extern sqInt slotSizeOf(sqInt oop);
extern sqInt stackIntegerValue(sqInt offset);
extern sqInt stackValue(sqInt offset);
extern sqInt success(sqInt aBoolean);

extern
#endif
struct VirtualMachine* interpreterProxy;
static const char *moduleName =
#ifdef SQUEAK_BUILTIN_PLUGIN
	"SerialPlugin VMMaker.oscog-eem.261 (i)"
#else
	"SerialPlugin VMMaker.oscog-eem.261 (e)"
#endif
;



/*	Note: This is coded so that plugins can be run from Squeak. */

static VirtualMachine *
getInterpreter(void)
{
	return interpreterProxy;
}


/*	Note: This is hardcoded so it can be run from Squeak.
	The module name is used for validating a module *after*
	it is loaded to check if it does really contain the module
	we're thinking it contains. This is important! */

EXPORT(const char*)
getModuleName(void)
{
	return moduleName;
}

static sqInt
halt(void)
{
	;
	return 0;
}

EXPORT(sqInt)
initialiseModule(void)
{
	return serialPortInit();
}

static sqInt
msg(char *s)
{
	fprintf(stderr, "\n%s: %s", moduleName, s);
	return 0;
}

EXPORT(sqInt)
primitiveSerialPortClose(void)
{
	sqInt portNum;

	portNum = stackIntegerValue(0);
	if (failed()) {
		return null;
	}
	serialPortClose(portNum);
	if (failed()) {
		return null;
	}
	pop(1);
	return null;
}

EXPORT(sqInt)
primitiveSerialPortOpen(void)
{
	sqInt baudRate;
	sqInt dataBits;
	sqInt inFlowControl;
	sqInt outFlowControl;
	sqInt parityType;
	sqInt portNum;
	sqInt stopBitsType;
	sqInt xOffChar;
	sqInt xOnChar;

	portNum = stackIntegerValue(8);
	baudRate = stackIntegerValue(7);
	stopBitsType = stackIntegerValue(6);
	parityType = stackIntegerValue(5);
	dataBits = stackIntegerValue(4);
	inFlowControl = stackIntegerValue(3);
	outFlowControl = stackIntegerValue(2);
	xOnChar = stackIntegerValue(1);
	xOffChar = stackIntegerValue(0);
	if (failed()) {
		return null;
	}
	serialPortOpen(
			portNum, baudRate, stopBitsType, parityType, dataBits,
			inFlowControl, outFlowControl, xOnChar, xOffChar);
	if (failed()) {
		return null;
	}
	pop(9);
	return null;
}

EXPORT(sqInt)
primitiveSerialPortOpenByName(void)
{
	sqInt baudRate;
	sqInt dataBits;
	sqInt inFlowControl;
	sqInt outFlowControl;
	sqInt parityType;
	char *port;
	char *portName;
	sqInt portNameSize;
	sqInt stopBitsType;
	sqInt xOffChar;
	sqInt xOnChar;

	success(isBytes(stackValue(8)));
	portName = ((char *) (firstIndexableField(stackValue(8))));
	baudRate = stackIntegerValue(7);
	stopBitsType = stackIntegerValue(6);
	parityType = stackIntegerValue(5);
	dataBits = stackIntegerValue(4);
	inFlowControl = stackIntegerValue(3);
	outFlowControl = stackIntegerValue(2);
	xOnChar = stackIntegerValue(1);
	xOffChar = stackIntegerValue(0);
	if (failed()) {
		return null;
	}
	portNameSize = slotSizeOf(((int) portName) - 4);
	port = calloc(portNameSize, sizeof(char));
	memcpy(port, portName, portNameSize);
	serialPortOpenByName(
			port, baudRate, stopBitsType, parityType, dataBits,
			inFlowControl, outFlowControl, xOnChar, xOffChar);
	free(port);
	if (failed()) {
		return null;
	}
	pop(9);
	return null;
}

EXPORT(sqInt)
primitiveSerialPortRead(void)
{
	char *array;
	sqInt arrayPtr;
	sqInt bytesRead;
	sqInt count;
	sqInt portNum;
	sqInt startIndex;
	sqInt _return_value;

	portNum = stackIntegerValue(3);
	success(isBytes(stackValue(2)));
	array = ((char *) (firstIndexableField(stackValue(2))));
	startIndex = stackIntegerValue(1);
	count = stackIntegerValue(0);
	if (failed()) {
		return null;
	}
	success((startIndex >= 1)
	 && (((startIndex + count) - 1) <= (byteSizeOf(((sqInt)(long)(array) - 4)))));
	arrayPtr = ((((sqInt)array)) + startIndex) - 1;
	bytesRead = serialPortReadInto( portNum, count, arrayPtr);
	_return_value = integerObjectOf(bytesRead);
	if (failed()) {
		return null;
	}
	popthenPush(5, _return_value);
	return null;
}

EXPORT(sqInt)
primitiveSerialPortReadByName(void)
{
	char *array;
	sqInt arrayPtr;
	sqInt bytesRead;
	sqInt count;
	char *port;
	char *portName;
	sqInt portNameSize;
	sqInt startIndex;
	sqInt _return_value;

	success(isBytes(stackValue(3)));
	portName = ((char *) (firstIndexableField(stackValue(3))));
	success(isBytes(stackValue(2)));
	array = ((char *) (firstIndexableField(stackValue(2))));
	startIndex = stackIntegerValue(1);
	count = stackIntegerValue(0);
	if (failed()) {
		return null;
	}
	success((startIndex >= 1)
	 && (((startIndex + count) - 1) <= (byteSizeOf(((sqInt)(long)(array) - 4)))));
	portNameSize = slotSizeOf(((int) portName) - 4);
	port = calloc(portNameSize, sizeof(char));
	memcpy(port, portName, portNameSize);
	arrayPtr = ((((sqInt)array)) + startIndex) - 1;
	bytesRead = serialPortReadIntoByName( port, count, arrayPtr);
	free(port);
	_return_value = integerObjectOf(bytesRead);
	if (failed()) {
		return null;
	}
	popthenPush(5, _return_value);
	return null;
}

EXPORT(sqInt)
primitiveSerialPortWrite(void)
{
	char *array;
	sqInt arrayPtr;
	sqInt bytesWritten;
	sqInt count;
	sqInt portNum;
	sqInt startIndex;
	sqInt _return_value;

	portNum = stackIntegerValue(3);
	success(isBytes(stackValue(2)));
	array = ((char *) (firstIndexableField(stackValue(2))));
	startIndex = stackIntegerValue(1);
	count = stackIntegerValue(0);
	if (failed()) {
		return null;
	}
	success((startIndex >= 1)
	 && (((startIndex + count) - 1) <= (byteSizeOf(((sqInt)(long)(array) - 4)))));
	if (!(failed())) {
		arrayPtr = ((((sqInt)array)) + startIndex) - 1;
		bytesWritten = serialPortWriteFrom(portNum, count, arrayPtr);
	}
	_return_value = integerObjectOf(bytesWritten);
	if (failed()) {
		return null;
	}
	popthenPush(5, _return_value);
	return null;
}

EXPORT(sqInt)
primitiveSerialPortWriteByName(void)
{
	char *array;
	sqInt arrayPtr;
	sqInt bytesWritten;
	sqInt count;
	char *port;
	char *portName;
	sqInt portNameSize;
	sqInt startIndex;
	sqInt _return_value;

	success(isBytes(stackValue(3)));
	portName = ((char *) (firstIndexableField(stackValue(3))));
	success(isBytes(stackValue(2)));
	array = ((char *) (firstIndexableField(stackValue(2))));
	startIndex = stackIntegerValue(1);
	count = stackIntegerValue(0);
	if (failed()) {
		return null;
	}
	portNameSize = slotSizeOf(((int) portName) - 4);
	port = calloc(portNameSize, sizeof(char));
	memcpy(port, portName, portNameSize);
	success((startIndex >= 1)
	 && (((startIndex + count) - 1) <= (byteSizeOf(((sqInt)(long)(array) - 4)))));
	if (!(failed())) {
		arrayPtr = ((((sqInt)array)) + startIndex) - 1;
		bytesWritten = serialPortWriteFromByName(port, count, arrayPtr);
	}
	free(port);
	_return_value = integerObjectOf(bytesWritten);
	if (failed()) {
		return null;
	}
	popthenPush(5, _return_value);
	return null;
}


/*	Note: This is coded so that it can be run in Squeak. */

EXPORT(sqInt)
setInterpreter(struct VirtualMachine*anInterpreter)
{
	sqInt ok;

	interpreterProxy = anInterpreter;
	ok = ((interpreterProxy->majorVersion()) == (VM_PROXY_MAJOR))
	 && ((interpreterProxy->minorVersion()) >= (VM_PROXY_MINOR));
	if (ok) {
		
#if !defined(SQUEAK_BUILTIN_PLUGIN)
		byteSizeOf = interpreterProxy->byteSizeOf;
		failed = interpreterProxy->failed;
		firstIndexableField = interpreterProxy->firstIndexableField;
		integerObjectOf = interpreterProxy->integerObjectOf;
		isBytes = interpreterProxy->isBytes;
		pop = interpreterProxy->pop;
		popthenPush = interpreterProxy->popthenPush;
		slotSizeOf = interpreterProxy->slotSizeOf;
		stackIntegerValue = interpreterProxy->stackIntegerValue;
		stackValue = interpreterProxy->stackValue;
		success = interpreterProxy->success;
#endif /* !defined(SQUEAK_BUILTIN_PLUGIN) */
	}
	return ok;
}

EXPORT(sqInt)
shutdownModule(void)
{
	return serialPortShutdown();
}

static void
sqAssert(sqInt aBool)
{
	/* missing DebugCode */;
}


#ifdef SQUEAK_BUILTIN_PLUGIN

void* SerialPlugin_exports[][3] = {
	{"SerialPlugin", "getModuleName", (void*)getModuleName},
	{"SerialPlugin", "initialiseModule", (void*)initialiseModule},
	{"SerialPlugin", "primitiveSerialPortClose", (void*)primitiveSerialPortClose},
	{"SerialPlugin", "primitiveSerialPortOpen", (void*)primitiveSerialPortOpen},
	{"SerialPlugin", "primitiveSerialPortOpenByName", (void*)primitiveSerialPortOpenByName},
	{"SerialPlugin", "primitiveSerialPortRead", (void*)primitiveSerialPortRead},
	{"SerialPlugin", "primitiveSerialPortReadByName", (void*)primitiveSerialPortReadByName},
	{"SerialPlugin", "primitiveSerialPortWrite", (void*)primitiveSerialPortWrite},
	{"SerialPlugin", "primitiveSerialPortWriteByName", (void*)primitiveSerialPortWriteByName},
	{"SerialPlugin", "setInterpreter", (void*)setInterpreter},
	{"SerialPlugin", "shutdownModule", (void*)shutdownModule},
	{NULL, NULL, NULL}
};

#endif /* ifdef SQ_BUILTIN_PLUGIN */
