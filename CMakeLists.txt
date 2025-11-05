cmake_minimum_required(VERSION 3.5)
project(motion)
find_package(OpenCV REQUIRED)

include_directories(/usr/local/include)
include_directories( ${OpenCV_INCLUDE_DIRS})

add_executable(motion motion.cpp)
target_link_libraries( motion ${OpenCV_LIBS})
target_link_libraries( motion pigpio pthread)
