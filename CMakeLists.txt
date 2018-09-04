# Based on https://github.com/mesosphere/serenity/blob/master/CMakeLists.txt
cmake_minimum_required(VERSION 3.0.0)

IF(CMAKE_COMPILER_IS_GNUCXX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

project(mesos-custom
    VERSION 0.1.0
    LANGUAGES CXX C)
enable_testing()

# Command line options.
set(WITH_MESOS "/usr/local" CACHE STRING "Mesos installation directory")
set(MESOS_INSTALLATION_DIR ${WITH_MESOS})
message("Mesos installation directory set to: " ${MESOS_INSTALLATION_DIR})

option(INTEGRATION_TESTS "Enable compilation of integration tests." OFF)

# Compiler things.
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
    message(STATUS "Error: ${CMAKE_CXX_COMPILER} has no C++11 support")
endif()

# Some libs needs to be compiled position indepenedent.
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

# Add gtest.
			add_subdirectory (${MESOS_SRC_DIR}/build/3rdparty/gmock-1.7.0)
#include_directories(${gmock_SOURCE_DIR} include)
#include_directories(${gtest_SOURCE_DIR} include)

set(LOGROTATE_CONTAINER_LOGGER_SRC lib_logrotate.cpp)
set(MESOS_LOGROTATE_LOGGER_SRC logrotate.cpp)
set(LOGROTATE_CONTAINER_LOGGER_TARGET logrotate_container_logger
  CACHE STRING "Library containing the logrotate container logger.")
set(MESOS_LOGROTATE_LOGGER_TARGET mesos-logrotate-logger
  CACHE STRING "Executable used by the logrotate container logger.")
set(EXECUTABLE_OUTPUT_PATH /usr/local/libexec/mesos)
set(LIBRARY_OUTPUT_PATH /usr/local/lib/mesos/modules)
set(MESOS_LIBS_TARGET ${MESOS_INSTALLATION_DIR}/lib)
  

link_directories(
    ${MESOS_INSTALLATION_DIR}/lib/
    ${MESOS_INSTALLATION_DIR}/lib/mesos/3rdparty/lib)

include_directories(
    ./
   
    ${MESOS_SRC_DIR}/src
    ${MESOS_INSTALLATION_DIR}/include/
    ${MESOS_INSTALLATION_DIR}/include/mesos
    ${MESOS_INSTALLATION_DIR}/lib
    ${MESOS_INSTALLATION_DIR}/lib/mesos/3rdparty/include
    ${CMAKE_BINARY_DIR}/
    $(CMAKE_INCLUDE_PATH)
)

add_definitions(
    
    -DPKGMODULEDIR="nowhere"
    -DLIBDIR="/usr/local/lib/mesos"
    -DPKGLIBEXECDIR="/usr/local/libexec/mesos"
    -DTESTLIBEXECDIR="nowhere"
    -DSBINDIR="/usr/local/sbin"
    -DPKGDATADIR="nowhere"
    -DVERSION="1.3.0"
)


set(MESOS_LIBS_TARGET ${MESOS_INSTALLATION_DIR}/lib)


# Module.
add_library(${LOGROTATE_CONTAINER_LOGGER_TARGET} SHARED lib_logrotate.cpp lib_logrotate.hpp logrotate.hpp inja.hpp json.hpp systemd.hpp)
add_executable(${MESOS_LOGROTATE_LOGGER_TARGET} ${MESOS_LOGROTATE_LOGGER_SRC} logrotate.hpp)


target_link_libraries(${MESOS_LOGROTATE_LOGGER_TARGET}
    m
    dl
    
    mesos

    glog
    systemd
)


target_link_libraries(${LOGROTATE_CONTAINER_LOGGER_TARGET}
    stdc++
    m
    dl
    
    mesos
 
    glog
    systemd
)



  add_dependencies(
    ${LOGROTATE_CONTAINER_LOGGER_TARGET} ${MESOS_LOGROTATE_LOGGER_TARGET})


