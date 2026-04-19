#include "ServoController.hpp"
#include <iostream>
#include <algorithm>

ServoController::ServoController() = default;

ServoController::~ServoController() {
    if (lgh_ >= 0) {
        center();
        lgGpiochipClose(lgh_);
    }
}

bool ServoController::init() {
    lgh_ = lgGpiochipOpen(0);
    if (lgh_ < 0) {
        std::cerr << "[Servo] lgGpiochipOpen failed: " << lgh_ << "\n";
        return false;
    }

    if (lgGpioClaimOutput(lgh_, 0, GPIO_SERVO, 0) < 0) {
        std::cerr << "[Servo] GPIO" << GPIO_SERVO << " claim failed\n";
        return false;
    }

    center();
    std::cout << "[Servo] GPIO" << GPIO_SERVO << " initialized\n";
    return true;
}

void ServoController::set(float duty) {
    const float clamped = std::clamp(duty, DUTY_MIN, DUTY_MAX);
    lgTxPwm(lgh_, GPIO_SERVO, FREQ_HZ, clamped, 0, 0);
}

void ServoController::center() {
    lgTxPwm(lgh_, GPIO_SERVO, FREQ_HZ, DUTY_NEUTRAL, 0, 0);
}
