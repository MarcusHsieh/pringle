#ifndef WORKERS_HPP
#define WORKERS_HPP

#include "main.hpp"
#include "MPU6050.h"

#include <opencv2/opencv.hpp>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include "RadioManager.hpp"

// declarations for worker threads
void motor_worker();
void camera_worker();
void listen_worker();

// pid function
struct PIDController {
    float Kp = 0.5f;
    float Ki = 0.01f;
    float Kd = 0.1f;

    float integral = 0.0f;
    float prev_error = 0.0f;

    float calculate(float setpoint, float current_val, float dt) {
        float error = setpoint - current_val;

        if (std::abs(error) < 1.0f) { return 0.0f; }

        integral += error * dt;
        float derivative = (error - prev_error) / dt;
        prev_error = error;

        return Kp * error + Ki * integral + Kd * derivative;
    }
};
#endif