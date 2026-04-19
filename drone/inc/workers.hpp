#ifndef WORKERS_HPP
#define WORKERS_HPP

#include "main.hpp"
#include "MPU6050.h"
#include "GamepadReader.hpp"

#include <opencv2/opencv.hpp>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <algorithm>
#include <cmath>

// Worker thread entry points
void motor_worker();
void camera_worker();
void gamepad_worker();

// Shared input state written by gamepad_worker, read by motor_worker
extern std::atomic<float> raw_fwd;    // [-1, 1], normalized, THROTTLE_CAP already applied
extern std::atomic<float> raw_turn;   // [-1, 1]
extern std::atomic<float> raw_servo;  // [-1, 1]

// Complementary-filtered PID for pitch stabilization
struct PIDController {
    float Kp = 0.3f;
    float Ki = 0.01f;
    float Kd = 0.05f;

    float integral   = 0.0f;
    float prev_error = 0.0f;

    static constexpr float DEADBAND = 1.0f;  // degrees — ignore small errors
    static constexpr float MAX_INT  = 5.0f;  // anti-windup clamp

    float calculate(float setpoint, float measured, float dt) {
        const float error = setpoint - measured;
        if (std::abs(error) < DEADBAND) {
            integral = 0.0f;
            return 0.0f;
        }
        integral   = std::clamp(integral + error * dt, -MAX_INT, MAX_INT);
        const float derivative = (error - prev_error) / dt;
        prev_error = error;
        return Kp * error + Ki * integral + Kd * derivative;
    }

    void reset() { integral = 0.0f; prev_error = 0.0f; }
};

// Rate limiter — smooths throttle setpoints to prevent lurch-induced pitch
struct StabilizationState {
    float ramped_fwd  = 0.0f;
    float ramped_turn = 0.0f;

    // Max change per second in normalized [-1, 1] space
    // 0.4/s means full-reverse to full-forward takes ~5 seconds
    static constexpr float MAX_RATE = 0.4f;

    void update(float target_fwd, float target_turn, float dt) {
        const float limit = MAX_RATE * dt;
        ramped_fwd  += std::clamp(target_fwd  - ramped_fwd,  -limit, limit);
        ramped_turn += std::clamp(target_turn - ramped_turn, -limit, limit);
    }
};

#endif
