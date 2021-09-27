cmake_minimum_required(VERSION 3.13)
project(md5model VERSION 1.0.0 DESCRIPTION "OpenGL 4.1 MD5 Model Library")
include_directories("../include")

######
# If you are on the Lab Machines, or have installed the OpenGL libraries somewhere
# other than on your path, leave the following two lines uncommented and update
# the path.  The current path is for the lab environments.
#
# If your installations are on your path, then comment these two lines
######
include_directories("/Users/carterfowler/Desktop/Comp_Sci/441/Resources/include")
link_directories(md5model PUBLIC "/Users/carterfowler/Desktop/Comp_Sci/441/Resources/lib")

add_library(md5model STATIC md5anim.cpp md5mesh.cpp)