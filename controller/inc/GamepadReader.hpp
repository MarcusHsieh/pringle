#pragma once
#include <string>
#include <cstdint>

struct GamepadState {
    float rightY;     // right stick vertical:   -1.0 = full reverse, +1.0 = full forward
    float rightX;     // right stick horizontal: -1.0 = full left,    +1.0 = full right
    float servoPos;   // d-pad vertical:         -1.0 = up, 0.0 = center, +1.0 = down
};

class GamepadReader {
public:
    explicit GamepadReader(const std::string& device = "");
    ~GamepadReader();

    bool init();
    void update(GamepadState& state);

private:
    bool tryOpen();

    std::string device_;
    int         fd_ = -1;

    static constexpr int AXIS_RIGHT_X = 3;
    static constexpr int AXIS_RIGHT_Y = 4;
    static constexpr int AXIS_DPAD_V  = 7;
};
