#ifndef MAIN_HPP
#define MAIN_HPP

#include <iostream>
#include <vector>
#include <string>
#include <lgpio.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>

extern int h;
extern bool running;

// Motor / servo pins (BCM numbering)
const int MOTOR_PIN_RIGHT = 26;
const int MOTOR_PIN_LEFT  = 20;
const int SERVO_PIN       = 21;

// Camera resolution
const int CAMERA_WIDTH  = 640;
const int CAMERA_HEIGHT = 480;

#endif
