#pragma once
#include <atomic>
#include <thread>
#include "VideoDisplay.hpp"

// Single-purpose controller: display the 5.8 GHz FPV video feed on the HDMI screen.
// All robot control is handled by the drone RPi via USB gamepad dongle.
class Controller {
public:
    Controller();
    bool init();
    void run();
    void stop();

private:
    void videoLoop();

    VideoDisplay video_;
    std::atomic<bool> running_{false};
    bool videoOk_{false};
};
