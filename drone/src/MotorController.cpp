#include "MotorController.hpp"
#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

MotorController::MotorController() = default;

MotorController::~MotorController() {
    if (lgh_ >= 0) {
        neutral();
        lgGpiochipClose(lgh_);
    }
}

bool MotorController::init() {
    lgh_ = lgGpiochipOpen(0);
    if (lgh_ < 0) {
        std::cerr << "[Motors] lgGpiochipOpen failed: " << lgh_ << "\n";
        return false;
    }

    if (lgGpioClaimOutput(lgh_, 0, GPIO_LEFT,  0) < 0 ||
        lgGpioClaimOutput(lgh_, 0, GPIO_RIGHT, 0) < 0) {
        std::cerr << "[Motors] GPIO claim failed — pin already in use?\n";
        return false;
    }

    neutral();
    std::cout << "[Motors] GPIO" << GPIO_LEFT << " (left) + GPIO"
              << GPIO_RIGHT << " (right) initialized\n";
    return true;
}

void MotorController::setLeft(float duty) {
    lgTxPwm(lgh_, GPIO_LEFT, FREQ_HZ, clamp(duty), 0, 0);
}

void MotorController::setRight(float duty) {
    lgTxPwm(lgh_, GPIO_RIGHT, FREQ_HZ, clamp(duty), 0, 0);
}

void MotorController::neutral() {
    lgTxPwm(lgh_, GPIO_LEFT,  FREQ_HZ, DUTY_NEUTRAL, 0, 0);
    lgTxPwm(lgh_, GPIO_RIGHT, FREQ_HZ, DUTY_NEUTRAL, 0, 0);
}

void MotorController::arm() {
    std::cout << "[Motors] Sending neutral (7.5%). Plug in LiPo NOW if not already...\n";
    for (int i = 0; i < 40; ++i) {
        neutral();
        std::this_thread::sleep_for(100ms);
        if (i == 20) std::cout << "[Motors] Still waiting for ESC to arm...\n";
    }
    std::cout << "[Motors] Armed — ESC should have beeped twice\n";
}

float MotorController::clamp(float duty) {
    return std::clamp(duty, DUTY_MIN, DUTY_MAX);
}
