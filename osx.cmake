include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/extracted/vm/include/osx
    ${CMAKE_CURRENT_SOURCE_DIR}/extracted/vm/include/unix
    ${CMAKE_CURRENT_SOURCE_DIR}/extracted/vm/include/common
)

set(EXTRACTED_SOURCES
#Common sources
    ${CMAKE_CURRENT_SOURCE_DIR}/extracted/vm/src/common/sqHeapMap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/extracted/vm/src/common/sqVirtualMachine.c
    ${CMAKE_CURRENT_SOURCE_DIR}/extracted/vm/src/common/sqNamedPrims.c
    ${CMAKE_CURRENT_SOURCE_DIR}/extracted/vm/src/common/sqExternalSemaphores.c
    ${CMAKE_CURRENT_SOURCE_DIR}/extracted/vm/src/common/sqTicker.c

#Platform sources
    ${CMAKE_CURRENT_SOURCE_DIR}/extracted/vm/src/unix/aio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/extracted/vm/src/unix/sqUnixHeartbeat.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/debugUnix.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utilsMac.mm

#Virtual Memory functions
    ${CMAKE_CURRENT_SOURCE_DIR}/src/memoryUnix.c
)

set(VM_FRONTEND_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/parameters.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/macOpenFileDialog.mm)

configure_file(resources/mac/Info.plist.in build/includes/Info.plist)

macro(add_third_party_dependencies_per_platform)
    add_third_party_dependency("pixman-0.34.0" ${LIBRARY_OUTPUT_DIRECTORY})
    add_third_party_dependency("cairo-1.15.4" ${LIBRARY_OUTPUT_DIRECTORY})
    add_third_party_dependency("freetype-2.9.1" ${LIBRARY_OUTPUT_DIRECTORY})
    add_third_party_dependency("libffi-3.3-rc0" ${LIBRARY_OUTPUT_DIRECTORY})
    add_third_party_dependency("libgit2-0.25.1" ${LIBRARY_OUTPUT_DIRECTORY})
    add_third_party_dependency("libpng-1.2.49" ${LIBRARY_OUTPUT_DIRECTORY})
    add_third_party_dependency("libssh2-1.7.0" ${LIBRARY_OUTPUT_DIRECTORY})
    add_third_party_dependency("openssl-1.0.2q" ${LIBRARY_OUTPUT_DIRECTORY})
    add_third_party_dependency("PThreadedFFI-1.1.1-osx64" ${LIBRARY_OUTPUT_DIRECTORY})
    add_third_party_dependency("SDL2-2.0.7" ${LIBRARY_OUTPUT_DIRECTORY})
endmacro()

macro(configure_installables INSTALL_COMPONENT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/build/dist")
    

endmacro()

macro(add_required_libs_per_platform)
   target_link_libraries(${VM_LIBRARY_NAME} "-framework AppKit")

   target_link_libraries(${VM_EXECUTABLE_NAME} "-framework AppKit")
   target_link_libraries(${VM_EXECUTABLE_NAME} "-framework CoreGraphics")
endmacro()