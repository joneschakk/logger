project(mesos_custom_logger)

set (MESOS_CONTAINER_LOGGER_MODULE_NAME "com_flytxt_mesos_LogShipContainerLogger")
set (LOGGER_COMPANION_BINARY_TARGET "logger-companion")
set (MESOS_CONTAINER_LOGGER_MODULE_TARGET "logship_container_logger")

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPKGLIBEXECDIR=\"/opt/mesos\"" ) 

message("Your C++ compiler supports these C++ features:")
foreach(i ${CMAKE_CXX_COMPILE_FEATURES})
  message("${i}")
endforeach()

if( (CMAKE_COMPILER_IS_GNUCXX AND NOT ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 4.6) OR
    (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 3.1))   
    message(STATUS "Checking for C++11 compiler - available")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
    message(FATAL_ERROR  "ERROR : C++11 compiler - unavailable")
endif()

#if(COMPILER_SUPPORTS_CXX11)
#   set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#else()
#   message (STATUS "Error: ${CMAKE_CXX_COMPILER} has no C++11 support")
#endif()

# Some libs needs to be compiled position indepenedent.
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

add_definitions(
   # -DSOURCE_DIR="${MESOS_SRC_DIR}/src"
   # -DBUILD_DIR="${MESOS_SRC_DIR}/build"
    -DPKGDATADIR="nowhere"
    -DVERSION="1.3.0"
    -DMODULE_NAME=${MESOS_CONTAINER_LOGGER_MODULE_NAME}
)

include_directories(
    ./
    /usr/include/
    /usr/lib/mesos/3rdparty/include 
)

link_directories(
    /usr/lib/
    /usr/lib/mesos/3rdparty/lib
)

######################
add_library(${MESOS_CONTAINER_LOGGER_MODULE_TARGET}  SHARED lib_logship.cpp)
target_link_libraries(${MESOS_CONTAINER_LOGGER_MODULE_TARGET}  PRIVATE mesos)

  # NOTE: This does not link to the above library because this is a
  # light-weight companion binary which can be run independently.
target_link_libraries(${MESOS_CONTAINER_LOGGER_MODULE_TARGET}
   # m
   # dl
    mesos
   # systemd
)

#########################
add_executable(${LOGGER_COMPANION_BINARY_TARGET}  companion.cpp)
target_link_libraries(${LOGGER_COMPANION_BINARY_TARGET}   
# stdc++
#  glog
  mesos 
)

# target_compile_options(${LOGGER_COMPANION_BINARY_TARGET}
# 
#)
set_target_properties(${LOGGER_COMPANION_BINARY_TARGET}  
	PROPERTIES LINK_FLAGS "-Wl,-warn-unresolved-symbols"
)

add_dependencies(
   ${LOGGER_COMPANION_BINARY_TARGET}
   ${MESOS_CONTAINER_LOGGER_MODULE_TARGET}
)

