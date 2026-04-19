cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(pringle_drone)

# Define the executable name
set(BINARY_NAME pringle_drone)

# 1. Include the 'inc' directory so the compiler finds main.hpp and workers.hpp
include_directories(inc)
include_directories(${CMAKE_SOURCE_DIR}/shared/)

find_package(OpenCV REQUIRED)

find_library(RF24_LIBRARY NAMES rf24 PATHS /usr/local/lib REQUIRED)
find_path(RF24_INCLUDE_DIR NAMES RF24.h PATHS /usr/local/include/RF24 REQUIRED)

# 2. Gather all the source files from 'src'
file(GLOB SOURCES "src/*.cpp")

# 3. Create the executable
add_executable(${BINARY_NAME} ${SOURCES})

# 4. Link Libraries
# Assuming you are using lgpio for the motors and OpenCV for the drone's CV tasks
target_include_directories(pringle_drone PRIVATE
    inc
    ${CMAKE_SOURCE_DIR}/..
    ${OpenCV_INCLUDE_DIRS}
    ${RF24_INCLUDE_DIR}
)


target_link_libraries(${BINARY_NAME} 
    PRIVATE 
    lgpio
    pthread
    m
    i2c
    ${RF24_LIBRARY}
    ${OpenCV_LIBS}
)

# Optional: Set the output directory to the root for convenience
set_target_properties(${BINARY_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")