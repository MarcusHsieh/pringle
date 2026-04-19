#include "Controller.hpp"
#include <iostream>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

Controller::Controller() = default;

bool Controller::init() {
    videoOk_ = video_.init();
    if (!videoOk_) std::cerr << "[Controller] Video unavailable — check Avedio USB capture on /dev/video0\n";
    running_ = true;
    return true;
}

void Controller::stop() {
    running_ = false;
}

void Controller::run() {
    videoLoop();
}

void Controller::videoLoop() {
    while (running_) {
        if (videoOk_) video_.update();
        else          std::this_thread::sleep_for(100ms);
    }
}
