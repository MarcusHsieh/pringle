cmake_minimum_required(VERSION 3.16)
project(pringle_controller LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

add_executable(pringle_controller
    src/main.cpp
    src/Controller.cpp
    src/VideoDisplay.cpp
)

target_include_directories(pringle_controller PRIVATE
    inc
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(pringle_controller PRIVATE
    ${OpenCV_LIBS}
    pthread
)
