cmake_minimum_required(VERSION 3.5)
project(CVersion)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)

find_package( OpenCV REQUIRED )

add_executable(CVersion ${SOURCE_FILES})

target_link_libraries( CVersion ${OpenCV_LIBS} )

