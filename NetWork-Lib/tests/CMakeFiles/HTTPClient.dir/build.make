# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.12

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


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

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/lixuhao/myprojects/mywebserver/NetWork-Lib

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/lixuhao/myprojects/mywebserver/NetWork-Lib

# Include any dependencies generated for this target.
include tests/CMakeFiles/HTTPClient.dir/depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/HTTPClient.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/HTTPClient.dir/flags.make

tests/CMakeFiles/HTTPClient.dir/HTTPClient.o: tests/CMakeFiles/HTTPClient.dir/flags.make
tests/CMakeFiles/HTTPClient.dir/HTTPClient.o: tests/HTTPClient.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/lixuhao/myprojects/mywebserver/NetWork-Lib/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object tests/CMakeFiles/HTTPClient.dir/HTTPClient.o"
	cd /home/lixuhao/myprojects/mywebserver/NetWork-Lib/tests && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/HTTPClient.dir/HTTPClient.o -c /home/lixuhao/myprojects/mywebserver/NetWork-Lib/tests/HTTPClient.cpp

tests/CMakeFiles/HTTPClient.dir/HTTPClient.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/HTTPClient.dir/HTTPClient.i"
	cd /home/lixuhao/myprojects/mywebserver/NetWork-Lib/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/lixuhao/myprojects/mywebserver/NetWork-Lib/tests/HTTPClient.cpp > CMakeFiles/HTTPClient.dir/HTTPClient.i

tests/CMakeFiles/HTTPClient.dir/HTTPClient.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/HTTPClient.dir/HTTPClient.s"
	cd /home/lixuhao/myprojects/mywebserver/NetWork-Lib/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/lixuhao/myprojects/mywebserver/NetWork-Lib/tests/HTTPClient.cpp -o CMakeFiles/HTTPClient.dir/HTTPClient.s

# Object files for target HTTPClient
HTTPClient_OBJECTS = \
"CMakeFiles/HTTPClient.dir/HTTPClient.o"

# External object files for target HTTPClient
HTTPClient_EXTERNAL_OBJECTS =

tests/HTTPClient: tests/CMakeFiles/HTTPClient.dir/HTTPClient.o
tests/HTTPClient: tests/CMakeFiles/HTTPClient.dir/build.make
tests/HTTPClient: tests/CMakeFiles/HTTPClient.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/lixuhao/myprojects/mywebserver/NetWork-Lib/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable HTTPClient"
	cd /home/lixuhao/myprojects/mywebserver/NetWork-Lib/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/HTTPClient.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/HTTPClient.dir/build: tests/HTTPClient

.PHONY : tests/CMakeFiles/HTTPClient.dir/build

tests/CMakeFiles/HTTPClient.dir/clean:
	cd /home/lixuhao/myprojects/mywebserver/NetWork-Lib/tests && $(CMAKE_COMMAND) -P CMakeFiles/HTTPClient.dir/cmake_clean.cmake
.PHONY : tests/CMakeFiles/HTTPClient.dir/clean

tests/CMakeFiles/HTTPClient.dir/depend:
	cd /home/lixuhao/myprojects/mywebserver/NetWork-Lib && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/lixuhao/myprojects/mywebserver/NetWork-Lib /home/lixuhao/myprojects/mywebserver/NetWork-Lib/tests /home/lixuhao/myprojects/mywebserver/NetWork-Lib /home/lixuhao/myprojects/mywebserver/NetWork-Lib/tests /home/lixuhao/myprojects/mywebserver/NetWork-Lib/tests/CMakeFiles/HTTPClient.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tests/CMakeFiles/HTTPClient.dir/depend

