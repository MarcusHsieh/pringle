#ifndef MAIN_HPP
#define MAIN_HPP

// basic headers
#include <iostream>
#include <vector>
#include <string>

// gpio control
#include <lgpio.h> 

// threading
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>

// gpio control handle
extern int h;

// running control flag
extern bool running;

// gpio config
namespace Config {
    constexpr int MOTOR_PIN_A =     18;
    constexpr int MOTOR_PIN_B =     33;
    constexpr int CAMERA_WIDTH =    1280;
    constexpr int CAMERA_HEIGHT =   720;
}

#endif