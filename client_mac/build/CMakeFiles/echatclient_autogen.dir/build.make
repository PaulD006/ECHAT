# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 4.0

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
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/paul/ENCCHAT/ECHAT/client_mac

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/paul/ENCCHAT/ECHAT/client_mac/build

# Utility rule file for echatclient_autogen.

# Include any custom commands dependencies for this target.
include CMakeFiles/echatclient_autogen.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/echatclient_autogen.dir/progress.make

CMakeFiles/echatclient_autogen: echatclient_autogen/timestamp

echatclient_autogen/timestamp: /usr/local/share/qt/libexec/moc
echatclient_autogen/timestamp: CMakeFiles/echatclient_autogen.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --blue --bold --progress-dir=/Users/paul/ENCCHAT/ECHAT/client_mac/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Automatic MOC for target echatclient"
	/usr/local/bin/cmake -E cmake_autogen /Users/paul/ENCCHAT/ECHAT/client_mac/build/CMakeFiles/echatclient_autogen.dir/AutogenInfo.json ""
	/usr/local/bin/cmake -E touch /Users/paul/ENCCHAT/ECHAT/client_mac/build/echatclient_autogen/timestamp

CMakeFiles/echatclient_autogen.dir/codegen:
.PHONY : CMakeFiles/echatclient_autogen.dir/codegen

echatclient_autogen: CMakeFiles/echatclient_autogen
echatclient_autogen: echatclient_autogen/timestamp
echatclient_autogen: CMakeFiles/echatclient_autogen.dir/build.make
.PHONY : echatclient_autogen

# Rule to build all files generated by this target.
CMakeFiles/echatclient_autogen.dir/build: echatclient_autogen
.PHONY : CMakeFiles/echatclient_autogen.dir/build

CMakeFiles/echatclient_autogen.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/echatclient_autogen.dir/cmake_clean.cmake
.PHONY : CMakeFiles/echatclient_autogen.dir/clean

CMakeFiles/echatclient_autogen.dir/depend:
	cd /Users/paul/ENCCHAT/ECHAT/client_mac/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/paul/ENCCHAT/ECHAT/client_mac /Users/paul/ENCCHAT/ECHAT/client_mac /Users/paul/ENCCHAT/ECHAT/client_mac/build /Users/paul/ENCCHAT/ECHAT/client_mac/build /Users/paul/ENCCHAT/ECHAT/client_mac/build/CMakeFiles/echatclient_autogen.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/echatclient_autogen.dir/depend

