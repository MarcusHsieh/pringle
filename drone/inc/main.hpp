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
    const int MOTOR_PIN_RIGHT =     26;
    const int MOTOR_PIN_LEFT =      20; 
    const int SERVO_PIN =           21;
    const int CAMERA_WIDTH =        1280; 
    const int CAMERA_HEIGHT =       720;

#endif