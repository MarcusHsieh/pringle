#include "GamepadReader.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>
#include <iostream>
#include <cerrno>
#include <cstring>

GamepadReader::GamepadReader(const std::string& device) : device_(device) {}

GamepadReader::~GamepadReader() {
    if (fd_ >= 0) ::close(fd_);
}

bool GamepadReader::init() {
    fd_ = ::open(device_.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd_ < 0) {
        std::cerr << "[Gamepad] Cannot open " << device_ << ": " << strerror(errno) << "\n";
        return false;
    }
    std::cout << "[Gamepad] Opened " << device_ << "\n";
    return true;
}

void GamepadReader::update(GamepadState& state) {
    js_event event;

    while (::read(fd_, &event, sizeof(event)) == static_cast<ssize_t>(sizeof(event))) {
        const uint8_t type = event.type & ~JS_EVENT_INIT;
        if (type != JS_EVENT_AXIS) continue;

        const float v = static_cast<float>(event.value) / 32767.0f;

        switch (event.number) {
            case AXIS_RIGHT_Y:
                state.rightY = -v;
                break;
            case AXIS_RIGHT_X:
                state.rightX = v;
                break;
            case AXIS_DPAD_V:
                state.servoPos = v;
                break;
            default:
                break;
        }
    }
}
