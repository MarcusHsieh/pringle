cmake_minimum_required(VERSION 3.16)
project(r6_controller LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
find_package(RF24 REQUIRED)

add_executable(r6_controller
    src/main.cpp
    # src/Controller.cpp
    # src/RadioManager.cpp
    # src/GamepadReader.cpp
    # src/VideoDisplay.cpp
)

target_include_directories(r6_controller PRIVATE
    include
    ${CMAKE_SOURCE_DIR}/..
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(r6_controller PRIVATE
    ${OpenCV_LIBS}
    RF24
    lgpio
    pthread
)
