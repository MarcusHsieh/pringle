cmake_minimum_required(VERSION 3.16)
project(r6_controller LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

# RF24 1.6.0 doesn't install a cmake config file — locate manually
find_library(RF24_LIBRARY NAMES rf24 PATHS /usr/local/lib REQUIRED)
find_path(RF24_INCLUDE_DIR NAMES RF24.h PATHS /usr/local/include/RF24 REQUIRED)

message(STATUS "RF24 library: ${RF24_LIBRARY}")
message(STATUS "RF24 include: ${RF24_INCLUDE_DIR}")

add_executable(r6_controller
    src/main.cpp
    src/Controller.cpp
    src/RadioManager.cpp
    src/GamepadReader.cpp
    src/VideoDisplay.cpp
)

target_include_directories(r6_controller PRIVATE
    include
    ${CMAKE_SOURCE_DIR}/..
    ${OpenCV_INCLUDE_DIRS}
    ${RF24_INCLUDE_DIR}
)

target_link_libraries(r6_controller PRIVATE
    ${OpenCV_LIBS}
    ${RF24_LIBRARY}
    lgpio
    pthread
)
