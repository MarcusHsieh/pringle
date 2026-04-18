# Define the executable name
set(BINARY_NAME pringle_drone)

# 1. Include the 'inc' directory so the compiler finds main.hpp and workers.hpp
include_directories(inc)
include_directories(${CMAKE_SOURCE_DIR})

# 2. Gather all the source files from 'src'
file(GLOB SOURCES "src/*.cpp")

# 3. Create the executable
add_executable(${BINARY_NAME} ${SOURCES})

# 4. Link Libraries
# Assuming you are using lgpio for the motors and OpenCV for the drone's CV tasks
target_link_libraries(${BINARY_NAME} 
    PRIVATE 
    ${OpenCV_LIBS}
    lgpio
    pthread
    m
    i2c
    rf24
)

# Optional: Set the output directory to the root for convenience
set_target_properties(${BINARY_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")