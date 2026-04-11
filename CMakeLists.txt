# cmake build file for liquid-wlan
cmake_minimum_required(VERSION 3.10)

# parse version number from include/liquid-wlan.h
include(cmake/version.cmake)
get_version("${CMAKE_CURRENT_SOURCE_DIR}/include/liquid-wlan.h" LIQUID_VERSION)

# project definition
project(liquid-wlan VERSION ${LIQUID_VERSION} LANGUAGES C CXX)

# ---------------------------------------- options ----------------------------------------

option(FIND_FFTW        "Set to ON to find FFTW library"                ON)
option(BUILD_PYTHON     "Set to ON to build python bindings"            OFF)
option(BUILD_LIQUID     "Set to ON to fetch/build liquid separately"    ON)
#option(BUILD_EXAMPLES   "Set to ON to build examples"           ON)
#option(BUILD_AUTOTESTS  "Set to ON to build autotest program"   ON)
#option(BUILD_BENCHMARKS "Set to ON to build benchmark program"  ON)
#option(ENABLE_SIMD      "Set to ON to build SIMD extensions"    ON)
#option(FIND_SIMD        "Set to ON to find SIMD extensions"     ON)
#option(BUILD_SANDBOX    "Set to ON to build sandbox programs"   OFF)
#option(BUILD_DOC        "Set to ON to build documentation"      OFF)
#option(COVERAGE         "Set to ON to build with code coverage" OFF)
#option(PROFILE        "Set to ON to build with profiling"     OFF)

# add 'cmake' subdirectory available for custom scripts
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# target_sources() command converts relative paths to absolute.
cmake_policy(SET CMP0076 NEW)

# main library name
set(LIBNAME liquidwlan)

# require a C99 compiler for all targets
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 14 CACHE STRING "C++ version selection")  # or 11, 14, 17, 20
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # optional, ensure standard is supported
#set(CMAKE_CXX_EXTENSIONS OFF)  # optional, keep compiler extensions off

include (CheckIncludeFile)
include (CheckFunctionExists)
include (CheckLibraryExists)
include (CheckSymbolExists)

# check external libraries
#variable_watch(fftw3f_INCLUDE_DIR)
if (FIND_FFTW)
    find_package(fftw3f)
endif()

if (BUILD_LIQUID)
    # dynamically fetch content
    include(FetchContent)
    FetchContent_Declare(
        liquid
        GIT_REPOSITORY https://github.com/jgaeddert/liquid-dsp.git
        GIT_TAG        master #v1.7.0
        )
    set(BUILD_AUTOTESTS  OFF CACHE INTERNAL "Disable building liquid tests")
    set(BUILD_BENCHMARKS OFF CACHE INTERNAL "Disable building liquid benchmarks")
    set(BUILD_EXAMPLES   OFF CACHE INTERNAL "Disable building liquid examples")
    FetchContent_MakeAvailable(liquid)
    # FetchContent_Populate(liquid) <- older policy, see CMP0169
else()
    # check for local installation
    find_package(liquid REQUIRED)
endif(BUILD_LIQUID)

# global flags
if (MSVC)
    # warning level 4
    add_compile_options(/W4)
else()
    # additional warnings
    add_compile_options(-Wall -fPIC -O3 -Wno-deprecated -Wno-deprecated-declarations)
endif()

include (CheckTypeSize)
check_type_size(int SIZEOF_INT LANGUAGE C)

# always enable testing
enable_testing()

#configure_file(cmake/config.h.cmake config.h)

# this file can only use the @VARIABLE@ syntax
#configure_file(cmake/version.c.in ${PROJECT_BINARY_DIR}/src/version.c @ONLY)

#target_include_directories(test PRIVATE include)
include_directories(include .)

# ---------------------------------------- main library ----------------------------------------

add_library(${LIBNAME} SHARED
    src/libliquid_wlan.c
	src/wlan_data_scrambler.c
	src/wlan_fec.c
	src/wlan_interleaver.c
	src/wlan_lfsr.c
	src/wlan_modem.c
	src/wlan_packet.c
	src/wlan_signal.c
	src/wlanframe.common.c
	src/wlanframegen.c
	src/wlanframesync.c
	src/utility.c
	src/wlan_intlv_R6.c
	src/wlan_intlv_R9.c
	src/wlan_intlv_R12.c
	src/wlan_intlv_R18.c
	src/wlan_intlv_R24.c
	src/wlan_intlv_R36.c
	src/wlan_intlv_R48.c
	src/wlan_intlv_R54.c
	src/libfec/viterbi27.c
	src/libfec/viterbi27_port.c
    )

# necessary to include this for the generated "config.h" header
include_directories(${CMAKE_CURRENT_BINARY_DIR})

target_include_directories(${LIBNAME} PRIVATE ${INCLUDE})
#target_sources(${LIBNAME} PUBLIC FILE_SET HEADERS FILES include/liquid-wlan.h)

target_link_libraries(${LIBNAME} c m liquid)
if (fftw3f_FOUND)
    target_link_libraries(${LIBNAME} fftw3f)
endif()

set_target_properties(${LIBNAME} PROPERTIES VERSION ${LIQUID_VERSION} SOVERSION 1)

# see: https://cmake.org/cmake/help/latest/command/install.html#examples
# TODO: uninstall?
install(TARGETS
          ${LIBNAME}
        RUNTIME           # Following options apply to runtime artifacts.
          COMPONENT Runtime
        LIBRARY           # Following options apply to library artifacts.
          COMPONENT Runtime
          NAMELINK_COMPONENT Development
        ARCHIVE           # Following options apply to archive artifacts.
          COMPONENT Development
          DESTINATION lib/static
        #FILE_SET HEADERS  # Following options apply to file set HEADERS.
          #COMPONENT Development
        )

install(FILES include/liquid-wlan.h DESTINATION include/liquid)

# examples
add_subdirectory(examples)

# sandbox
add_subdirectory(sandbox)

# autotest
add_subdirectory(autotest)

# benchmarks
# TODO

# documentation
# TODO

# packaging

set(CPACK_PACKAGE_NAME liquid-wlan)
set(CPACK_PACKAGE_VENDOR "Joseph D. Gaeddert")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Software-Defined Wi-Fi 802.11a/g Library")
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_NAME})
set(CPACK_VERBATIM_VARIABLES TRUE)
include(CPack)

# ---------------------------------------- python bindings ----------------------------------------

if(BUILD_PYTHON)
    set(PYBIND11_NEWPYTHON ON)
    find_package(pybind11 CONFIG REQUIRED)

    set(LIBNAME_PYTHON pyliquid_wlan)
    pybind11_add_module(${LIBNAME_PYTHON} MODULE
        bind/liquid-wlan.python.cc
        bind/wlanframegen.cc
        bind/wlanframesync.cc
        $<TARGET_OBJECTS:${LIBNAME}>
    )

    # Use different names for cmake target and output file since we cannot use
    # the cmake target 'liquid' more than once, but we also want the python package
    # to called 'liquid_wlan' and not something odd like 'pyliquid_wlan'
    # see: https://stackoverflow.com/questions/67212833/python-library-and-cmake-target-with-the-same-name#67226146
    set_target_properties(${LIBNAME_PYTHON} PROPERTIES OUTPUT_NAME liquid_wlan)

    # enable python extensions to C++ classes
    target_compile_definitions(${LIBNAME_PYTHON} PUBLIC LIQUID_PYTHONLIB)
    # pull version info from top of this file
    target_compile_definitions(${LIBNAME_PYTHON} PRIVATE VERSION_INFO="${PROJECT_VERSION}")

    target_link_libraries(${LIBNAME_PYTHON} PUBLIC c m liquid)
    if (fftw3f_FOUND)
        target_link_libraries(${LIBNAME_PYTHON} PUBLIC fftw3f)
    endif()

    install(TARGETS ${LIBNAME_PYTHON} LIBRARY DESTINATION .)
endif(BUILD_PYTHON)


# ---------------------------------------- summary ----------------------------------------

# debug cmake variables
#include(cmake/debug.cmake)

# cmake config summary
include(cmake/summary.cmake)
cmake_print_config_summary()

