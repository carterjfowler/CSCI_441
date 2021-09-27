cmake_minimum_required(VERSION 3.12)
project(A1)
set(CMAKE_CXX_STANDARD 11)
set(SOURCE_FILES main.cpp)
add_executable(A1 ${SOURCE_FILES})

######
# If you are on the Lab Machines, or have installed the OpenGL libraries somewhere
# other than on your path, leave the following two lines uncommented and update
# the path.  The current path is for the lab environments.
#
# If your installations are on your path, then comment these two lines
######

include_directories("/Users/carterfowler/Desktop/Comp_Sci/441/Resources/include")
target_link_directories(A1 PUBLIC "/Users/carterfowler/Desktop/Comp_Sci/441/Resources/lib")

# the following line is linking instructions for Windows.  comment if on OS X, otherwise leave uncommented
#target_link_libraries(lab01 opengl32 glfw3 glew32.dll gdi32)

# the following line is linking instructions for OS X.  uncomment if on OS X, otherwise leave commented
target_link_libraries(A1 "-framework OpenGL" glfw3 "-framework Cocoa" "-framework IOKit" "-framework CoreVideo" glew)