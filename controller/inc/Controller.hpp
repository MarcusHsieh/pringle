#pragma once
#include <thread>
#include <atomic>
#include "RadioManager.hpp"
#include "GamepadReader.hpp"
#include "VideoDisplay.hpp"
#include "shared/protocol.hpp"

// Three concurrent threads:
//
//  gamepadLoop  (~200 Hz, background thread)
//    reads joystick -> mixes -> writes duty% atomics
//
//  radioLoop  (40 Hz, background thread)
//    reads duty% atomics -> builds ControlPacket -> sends -> reads telemetry ACK
//
//  videoLoop  (main thread - OpenCV imshow must stay on main thread)
//    grabs frames from Avedio USB capture -> displays on screen
//
// Shared state between gamepad and radio loops: three atomic<float> duty values
class Controller {
public:
    Controller();
    bool init();
    void run();
    void stop();

private:
    void gamepadLoop();
    void radioLoop();
    void videoLoop();

    struct Duties { float left, right, servo; };
    Duties mixInputs(const GamepadState& g) const;

    // Hardware
    RadioManager  radio_;
    GamepadReader gamepad_;
    VideoDisplay  video_;

    // Shared state (gamepadLoop writes, radioLoop reads)
    std::atomic<float> leftDuty_{7.5f};
    std::atomic<float> rightDuty_{7.5f};
    std::atomic<float> servoDuty_{7.5f};

    std::atomic<bool> running_{false};
    bool videoOk_{false};

    static constexpr int   CONTROL_HZ   = 40;
    static constexpr float THROTTLE_CAP = 0.65f;   // fraction of 2.5% range -> 7.5 + 0.65*2.5 = 9.125 max duty
};
