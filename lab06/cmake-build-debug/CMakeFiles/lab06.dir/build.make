# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.17

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
CMAKE_COMMAND = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake

# The command to remove a file.
RM = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/carterfowler/Desktop/Comp_Sci/441/Projects/lab06

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/carterfowler/Desktop/Comp_Sci/441/Projects/lab06/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/lab06.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/lab06.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/lab06.dir/flags.make

CMakeFiles/lab06.dir/main.cpp.o: CMakeFiles/lab06.dir/flags.make
CMakeFiles/lab06.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/carterfowler/Desktop/Comp_Sci/441/Projects/lab06/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/lab06.dir/main.cpp.o"
	/Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/lab06.dir/main.cpp.o -c /Users/carterfowler/Desktop/Comp_Sci/441/Projects/lab06/main.cpp

CMakeFiles/lab06.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/lab06.dir/main.cpp.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/carterfowler/Desktop/Comp_Sci/441/Projects/lab06/main.cpp > CMakeFiles/lab06.dir/main.cpp.i

CMakeFiles/lab06.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/lab06.dir/main.cpp.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/carterfowler/Desktop/Comp_Sci/441/Projects/lab06/main.cpp -o CMakeFiles/lab06.dir/main.cpp.s

# Object files for target lab06
lab06_OBJECTS = \
"CMakeFiles/lab06.dir/main.cpp.o"

# External object files for target lab06
lab06_EXTERNAL_OBJECTS =

lab06: CMakeFiles/lab06.dir/main.cpp.o
lab06: CMakeFiles/lab06.dir/build.make
lab06: src/libstbimage.a
lab06: CMakeFiles/lab06.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/carterfowler/Desktop/Comp_Sci/441/Projects/lab06/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable lab06"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/lab06.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/lab06.dir/build: lab06

.PHONY : CMakeFiles/lab06.dir/build

CMakeFiles/lab06.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/lab06.dir/cmake_clean.cmake
.PHONY : CMakeFiles/lab06.dir/clean

CMakeFiles/lab06.dir/depend:
	cd /Users/carterfowler/Desktop/Comp_Sci/441/Projects/lab06/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/carterfowler/Desktop/Comp_Sci/441/Projects/lab06 /Users/carterfowler/Desktop/Comp_Sci/441/Projects/lab06 /Users/carterfowler/Desktop/Comp_Sci/441/Projects/lab06/cmake-build-debug /Users/carterfowler/Desktop/Comp_Sci/441/Projects/lab06/cmake-build-debug /Users/carterfowler/Desktop/Comp_Sci/441/Projects/lab06/cmake-build-debug/CMakeFiles/lab06.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/lab06.dir/depend

