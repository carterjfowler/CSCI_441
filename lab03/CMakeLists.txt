cmake_minimum_required(VERSION 3.13)
project(lab03)
set(CMAKE_CXX_STANDARD 11)
set(SOURCE_FILES main.cpp lab03.cpp)
add_executable(lab03 ${SOURCE_FILES})

add_subdirectory(src)

include_directories("include/")

######
# If you are on the Lab Machines, or have installed the OpenGL libraries somewhere
# other than on your path, leave the following two lines uncommented and update
# the path.  The current path is for the lab environments.
#
# If your installations are on your path, then comment these two lines
######

include_directories("/Users/carterfowler/Desktop/Comp_Sci/441/Resources/include")
target_link_directories(lab03 PUBLIC "/Users/carterfowler/Desktop/Comp_Sci/441/Resources/lib")

# the following line is linking instructions for Windows.  comment if on OS X, otherwise leave uncommented
#target_link_libraries(lab03 md5model opengl32 glfw3 glew32.dll gdi32)

# the following line is linking instructions for OS X.  uncomment if on OS X, otherwise leave commented
target_link_libraries(lab03 "-framework OpenGL" glfw3 "-framework Cocoa" "-framework IOKit" "-framework CoreVideo" glew md5model)
