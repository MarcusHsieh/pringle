cmake_minimum_required(VERSION 3.16)
project(r6_drone LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(RF24 REQUIRED)
find_package(OpenCV REQUIRED)

add_executable(r6_drone
    src/main.cpp
    # src/DroneApp.cpp
    # src/RadioManager.cpp
    # src/MotorController.cpp
    # src/ServoController.cpp
    # src/CameraCapture.cpp
)

target_include_directories(r6_drone PRIVATE
    include
    ${CMAKE_SOURCE_DIR}/..
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(r6_drone PRIVATE
    RF24
    lgpio
    ${OpenCV_LIBS}
    pthread
)
