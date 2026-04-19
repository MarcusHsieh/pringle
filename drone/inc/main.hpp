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

// motor pins (BCM numbering)
const int MOTOR_PIN_RIGHT =     26;
const int MOTOR_PIN_LEFT =      20; 
const int SERVO_PIN =           21;

// camera configuration
const int CAMERA_WIDTH =        640; 
const int CAMERA_HEIGHT =       480;

// NRF module pins (BCM numbering)
const int NRF_SCK_PIN = 11;
const int NRF_MOSI_PIN = 10;
const int NRF_MISO_PIN = 9;


#endif