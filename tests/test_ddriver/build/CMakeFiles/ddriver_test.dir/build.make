# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
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
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/meteor/user-land-filesystem/tests/test_ddriver

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/meteor/user-land-filesystem/tests/test_ddriver/build

# Include any dependencies generated for this target.
include CMakeFiles/ddriver_test.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/ddriver_test.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/ddriver_test.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/ddriver_test.dir/flags.make

CMakeFiles/ddriver_test.dir/src/test.c.o: CMakeFiles/ddriver_test.dir/flags.make
CMakeFiles/ddriver_test.dir/src/test.c.o: ../src/test.c
CMakeFiles/ddriver_test.dir/src/test.c.o: CMakeFiles/ddriver_test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/meteor/user-land-filesystem/tests/test_ddriver/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/ddriver_test.dir/src/test.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/ddriver_test.dir/src/test.c.o -MF CMakeFiles/ddriver_test.dir/src/test.c.o.d -o CMakeFiles/ddriver_test.dir/src/test.c.o -c /home/meteor/user-land-filesystem/tests/test_ddriver/src/test.c

CMakeFiles/ddriver_test.dir/src/test.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/ddriver_test.dir/src/test.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/meteor/user-land-filesystem/tests/test_ddriver/src/test.c > CMakeFiles/ddriver_test.dir/src/test.c.i

CMakeFiles/ddriver_test.dir/src/test.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/ddriver_test.dir/src/test.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/meteor/user-land-filesystem/tests/test_ddriver/src/test.c -o CMakeFiles/ddriver_test.dir/src/test.c.s

# Object files for target ddriver_test
ddriver_test_OBJECTS = \
"CMakeFiles/ddriver_test.dir/src/test.c.o"

# External object files for target ddriver_test
ddriver_test_EXTERNAL_OBJECTS =

ddriver_test: CMakeFiles/ddriver_test.dir/src/test.c.o
ddriver_test: CMakeFiles/ddriver_test.dir/build.make
ddriver_test: /home/meteor/lib/libddriver.a
ddriver_test: CMakeFiles/ddriver_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/meteor/user-land-filesystem/tests/test_ddriver/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable ddriver_test"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ddriver_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/ddriver_test.dir/build: ddriver_test
.PHONY : CMakeFiles/ddriver_test.dir/build

CMakeFiles/ddriver_test.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/ddriver_test.dir/cmake_clean.cmake
.PHONY : CMakeFiles/ddriver_test.dir/clean

CMakeFiles/ddriver_test.dir/depend:
	cd /home/meteor/user-land-filesystem/tests/test_ddriver/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/meteor/user-land-filesystem/tests/test_ddriver /home/meteor/user-land-filesystem/tests/test_ddriver /home/meteor/user-land-filesystem/tests/test_ddriver/build /home/meteor/user-land-filesystem/tests/test_ddriver/build /home/meteor/user-land-filesystem/tests/test_ddriver/build/CMakeFiles/ddriver_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/ddriver_test.dir/depend

