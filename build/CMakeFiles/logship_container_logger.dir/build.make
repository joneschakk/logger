# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = /usr/bin/ccmake

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /opt/mesos-custom-logger

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /opt/mesos-custom-logger/build

# Include any dependencies generated for this target.
include CMakeFiles/logship_container_logger.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/logship_container_logger.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/logship_container_logger.dir/flags.make

CMakeFiles/logship_container_logger.dir/lib_logship.o: CMakeFiles/logship_container_logger.dir/flags.make
CMakeFiles/logship_container_logger.dir/lib_logship.o: ../lib_logship.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /opt/mesos-custom-logger/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/logship_container_logger.dir/lib_logship.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/logship_container_logger.dir/lib_logship.o -c /opt/mesos-custom-logger/lib_logship.cpp

CMakeFiles/logship_container_logger.dir/lib_logship.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/logship_container_logger.dir/lib_logship.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /opt/mesos-custom-logger/lib_logship.cpp > CMakeFiles/logship_container_logger.dir/lib_logship.i

CMakeFiles/logship_container_logger.dir/lib_logship.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/logship_container_logger.dir/lib_logship.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /opt/mesos-custom-logger/lib_logship.cpp -o CMakeFiles/logship_container_logger.dir/lib_logship.s

CMakeFiles/logship_container_logger.dir/lib_logship.o.requires:
.PHONY : CMakeFiles/logship_container_logger.dir/lib_logship.o.requires

CMakeFiles/logship_container_logger.dir/lib_logship.o.provides: CMakeFiles/logship_container_logger.dir/lib_logship.o.requires
	$(MAKE) -f CMakeFiles/logship_container_logger.dir/build.make CMakeFiles/logship_container_logger.dir/lib_logship.o.provides.build
.PHONY : CMakeFiles/logship_container_logger.dir/lib_logship.o.provides

CMakeFiles/logship_container_logger.dir/lib_logship.o.provides.build: CMakeFiles/logship_container_logger.dir/lib_logship.o

# Object files for target logship_container_logger
logship_container_logger_OBJECTS = \
"CMakeFiles/logship_container_logger.dir/lib_logship.o"

# External object files for target logship_container_logger
logship_container_logger_EXTERNAL_OBJECTS =

liblogship_container_logger.so: CMakeFiles/logship_container_logger.dir/lib_logship.o
liblogship_container_logger.so: CMakeFiles/logship_container_logger.dir/build.make
liblogship_container_logger.so: CMakeFiles/logship_container_logger.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX shared library liblogship_container_logger.so"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/logship_container_logger.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/logship_container_logger.dir/build: liblogship_container_logger.so
.PHONY : CMakeFiles/logship_container_logger.dir/build

CMakeFiles/logship_container_logger.dir/requires: CMakeFiles/logship_container_logger.dir/lib_logship.o.requires
.PHONY : CMakeFiles/logship_container_logger.dir/requires

CMakeFiles/logship_container_logger.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/logship_container_logger.dir/cmake_clean.cmake
.PHONY : CMakeFiles/logship_container_logger.dir/clean

CMakeFiles/logship_container_logger.dir/depend:
	cd /opt/mesos-custom-logger/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /opt/mesos-custom-logger /opt/mesos-custom-logger /opt/mesos-custom-logger/build /opt/mesos-custom-logger/build /opt/mesos-custom-logger/build/CMakeFiles/logship_container_logger.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/logship_container_logger.dir/depend
