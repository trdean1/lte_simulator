cmake_minimum_required (VERSION 2.6)
project (IoT_Sim)

set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_CXX_STANDARD 11)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g")

add_subdirectory (Src)
add_subdirectory (Test)
add_subdirectory (Bin)
