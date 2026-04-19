cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(pringle_drone)

set(BINARY_NAME pringle_drone)

include_directories(inc)

find_package(OpenCV REQUIRED)

add_executable(${BINARY_NAME}
    src/main.cpp
    src/workers.cpp
    src/GamepadReader.cpp
    src/MPU6050.cpp
)

target_include_directories(${BINARY_NAME} PRIVATE
    inc
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(${BINARY_NAME} PRIVATE
    lgpio
    pthread
    m
    i2c
    ${OpenCV_LIBS}
)

set_target_properties(${BINARY_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
