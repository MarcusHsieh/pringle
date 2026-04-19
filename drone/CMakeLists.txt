cmake_minimum_required(VERSION 3.16)
project(pringle_drone LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

find_library(RF24_LIBRARY NAMES rf24 PATHS /usr/local/lib REQUIRED)
find_path(RF24_INCLUDE_DIR NAMES RF24.h PATHS /usr/local/include/RF24 REQUIRED)

message(STATUS "RF24 library: ${RF24_LIBRARY}")
message(STATUS "RF24 include: ${RF24_INCLUDE_DIR}")

file(GLOB SOURCES "src/*.cpp")

add_executable(pringle_drone ${SOURCES})

target_include_directories(pringle_drone PRIVATE
    include                    # our DroneApp, RadioManager, Motor, Servo, Camera headers
    inc                        # teammate's headers (main.hpp, workers.hpp) if present
    ${CMAKE_SOURCE_DIR}/..     # exposes shared/protocol.hpp
    ${OpenCV_INCLUDE_DIRS}
    ${RF24_INCLUDE_DIR}
)

target_link_libraries(pringle_drone PRIVATE
    ${OpenCV_LIBS}
    ${RF24_LIBRARY}
    lgpio
    pthread
    m
)
