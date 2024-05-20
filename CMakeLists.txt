cmake_minimum_required(VERSION 3.5)
project(VC_Project)

# Find OpenCV package
find_package(OpenCV REQUIRED)
# Include directories from OpenCV
include_directories(${OpenCV_INCLUDE_DIRS})

# Create an executable
add_executable(VC_Project main.cpp vc.c)

# Link OpenCV Libraries
target_link_libraries(VC_Project ${OpenCV_LIBS})

#set(CPACK_PROJECT_NAME ${PROJECT_NAME})
#set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
#include(CPack)