# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

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
CMAKE_SOURCE_DIR = /home/hgen/Desktop/glwarp

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/hgen/Desktop/glwarp/build

# Include any dependencies generated for this target.
include CMakeFiles/glwarp.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/glwarp.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/glwarp.dir/flags.make

CMakeFiles/glwarp.dir/main.cpp.o: CMakeFiles/glwarp.dir/flags.make
CMakeFiles/glwarp.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/hgen/Desktop/glwarp/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/glwarp.dir/main.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/glwarp.dir/main.cpp.o -c /home/hgen/Desktop/glwarp/main.cpp

CMakeFiles/glwarp.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/glwarp.dir/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/hgen/Desktop/glwarp/main.cpp > CMakeFiles/glwarp.dir/main.cpp.i

CMakeFiles/glwarp.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/glwarp.dir/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/hgen/Desktop/glwarp/main.cpp -o CMakeFiles/glwarp.dir/main.cpp.s

CMakeFiles/glwarp.dir/main.cpp.o.requires:

.PHONY : CMakeFiles/glwarp.dir/main.cpp.o.requires

CMakeFiles/glwarp.dir/main.cpp.o.provides: CMakeFiles/glwarp.dir/main.cpp.o.requires
	$(MAKE) -f CMakeFiles/glwarp.dir/build.make CMakeFiles/glwarp.dir/main.cpp.o.provides.build
.PHONY : CMakeFiles/glwarp.dir/main.cpp.o.provides

CMakeFiles/glwarp.dir/main.cpp.o.provides.build: CMakeFiles/glwarp.dir/main.cpp.o


CMakeFiles/glwarp.dir/src/shader.cpp.o: CMakeFiles/glwarp.dir/flags.make
CMakeFiles/glwarp.dir/src/shader.cpp.o: ../src/shader.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/hgen/Desktop/glwarp/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/glwarp.dir/src/shader.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/glwarp.dir/src/shader.cpp.o -c /home/hgen/Desktop/glwarp/src/shader.cpp

CMakeFiles/glwarp.dir/src/shader.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/glwarp.dir/src/shader.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/hgen/Desktop/glwarp/src/shader.cpp > CMakeFiles/glwarp.dir/src/shader.cpp.i

CMakeFiles/glwarp.dir/src/shader.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/glwarp.dir/src/shader.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/hgen/Desktop/glwarp/src/shader.cpp -o CMakeFiles/glwarp.dir/src/shader.cpp.s

CMakeFiles/glwarp.dir/src/shader.cpp.o.requires:

.PHONY : CMakeFiles/glwarp.dir/src/shader.cpp.o.requires

CMakeFiles/glwarp.dir/src/shader.cpp.o.provides: CMakeFiles/glwarp.dir/src/shader.cpp.o.requires
	$(MAKE) -f CMakeFiles/glwarp.dir/build.make CMakeFiles/glwarp.dir/src/shader.cpp.o.provides.build
.PHONY : CMakeFiles/glwarp.dir/src/shader.cpp.o.provides

CMakeFiles/glwarp.dir/src/shader.cpp.o.provides.build: CMakeFiles/glwarp.dir/src/shader.cpp.o


# Object files for target glwarp
glwarp_OBJECTS = \
"CMakeFiles/glwarp.dir/main.cpp.o" \
"CMakeFiles/glwarp.dir/src/shader.cpp.o"

# External object files for target glwarp
glwarp_EXTERNAL_OBJECTS =

glwarp: CMakeFiles/glwarp.dir/main.cpp.o
glwarp: CMakeFiles/glwarp.dir/src/shader.cpp.o
glwarp: CMakeFiles/glwarp.dir/build.make
glwarp: /usr/lib/x86_64-linux-gnu/libGL.so
glwarp: /usr/lib/x86_64-linux-gnu/libGLU.so
glwarp: /usr/lib/x86_64-linux-gnu/libGLEW.so
glwarp: /usr/lib/x86_64-linux-gnu/libSM.so
glwarp: /usr/lib/x86_64-linux-gnu/libICE.so
glwarp: /usr/lib/x86_64-linux-gnu/libX11.so
glwarp: /usr/lib/x86_64-linux-gnu/libXext.so
glwarp: CMakeFiles/glwarp.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/hgen/Desktop/glwarp/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable glwarp"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/glwarp.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/glwarp.dir/build: glwarp

.PHONY : CMakeFiles/glwarp.dir/build

CMakeFiles/glwarp.dir/requires: CMakeFiles/glwarp.dir/main.cpp.o.requires
CMakeFiles/glwarp.dir/requires: CMakeFiles/glwarp.dir/src/shader.cpp.o.requires

.PHONY : CMakeFiles/glwarp.dir/requires

CMakeFiles/glwarp.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/glwarp.dir/cmake_clean.cmake
.PHONY : CMakeFiles/glwarp.dir/clean

CMakeFiles/glwarp.dir/depend:
	cd /home/hgen/Desktop/glwarp/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/hgen/Desktop/glwarp /home/hgen/Desktop/glwarp /home/hgen/Desktop/glwarp/build /home/hgen/Desktop/glwarp/build /home/hgen/Desktop/glwarp/build/CMakeFiles/glwarp.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/glwarp.dir/depend
