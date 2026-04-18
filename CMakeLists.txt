cmake_minimum_required(VERSION 3.16)
project(pringle)

# Running controller/ and drone/ independently on their respective RPis
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Drone CMake

find_package(OpenCV REQUIRED)
add_subdirectory(drone)