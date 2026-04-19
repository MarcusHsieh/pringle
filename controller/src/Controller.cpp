#include "Controller.hpp"
#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>

using namespace std::chrono;
using namespace std::chrono_literals;

Controller::Controller() = default;

bool Controller::init() {
    if (!radio_.init())   { std::cerr << "[Controller] Radio init failed\n";   return false; }
    if (!gamepad_.init()) { std::cerr << "[Controller] Gamepad init failed\n"; return false; }
    if (!video_.init())   { std::cerr << "[Controller] Video init failed — running without display\n"; }
    running_ = true;
    std::cout << "[Controller] Ready — sending at " << CONTROL_HZ << " Hz\n";
    return true;
}

void Controller::stop() {
    running_ = false;
}

void Controller::run() {
    // gamepad and radio on background threads; video stays on main thread
    std::thread gt([this]{ gamepadLoop(); });
    std::thread rt([this]{ radioLoop();   });

    videoLoop();   // blocks on main thread until running_ = false

    gt.join();
    rt.join();
}

// ── gamepadLoop ──────────────────────────────────────────────────────────────
void Controller::gamepadLoop() {
    GamepadState g{};
    while (running_) {
        gamepad_.update(g);
        const auto d = mixInputs(g);
        leftDuty_.store(d.left);
        rightDuty_.store(d.right);
        servoDuty_.store(d.servo);
        std::this_thread::sleep_for(5ms);   // ~200 Hz
    }
}

// ── radioLoop ────────────────────────────────────────────────────────────────
void Controller::radioLoop() {
    constexpr auto period = microseconds(1'000'000 / CONTROL_HZ);
    auto next = steady_clock::now();

    TelemetryPacket telem{};
    int dropCount = 0;

    while (running_) {
        if (steady_clock::now() >= next) {
            const ControlPacket pkt{
                .leftDuty  = leftDuty_.load(),
                .rightDuty = rightDuty_.load(),
                .servoDuty = servoDuty_.load()
            };

            if (radio_.sendControl(pkt, &telem)) {
                dropCount = 0;
                // Telemetry available — uncomment to log:
                // std::printf("TEMP:%3d°C  SEQ:%3d  FLAGS:%02x\n",
                //             telem.cpuTemp, telem.seq, telem.flags);
            } else {
                if (++dropCount % 20 == 0) {
                    std::cerr << "[Radio] " << dropCount << " consecutive drops\n";
                }
            }
            next += period;
        }
        std::this_thread::sleep_for(1ms);
    }
}

// ── videoLoop ────────────────────────────────────────────────────────────────
void Controller::videoLoop() {
    while (running_) {
        video_.update();   // grab + imshow + waitKey(1) — must stay on main thread
    }
}

// ── mixInputs ────────────────────────────────────────────────────────────────
// Differential drive: left = fwd + turn, right = fwd - turn.
// THROTTLE_CAP limits range to 65% of [5.0, 10.0] so the drone doesn't
// immediately rocket. Raise it once you're comfortable with the handling.
Controller::Duties Controller::mixInputs(const GamepadState& g) const {
    const float fwd  = g.rightY * THROTTLE_CAP;   // [-0.65, +0.65]
    const float turn = g.rightX * THROTTLE_CAP;

    const float left  = std::clamp(fwd + turn, -1.0f, 1.0f);
    const float right = std::clamp(fwd - turn, -1.0f, 1.0f);

    // Map [-1, 1] → [5.0, 10.0]%
    // neutral (0.0) → 7.5%,  full forward (1.0) → 10.0%,  full reverse (-1.0) → 5.0%
    auto toDuty = [](float v) -> float {
        return 7.5f + v * 2.5f;
    };

    return Duties{ toDuty(left), toDuty(right), toDuty(std::clamp(g.servoPos, -1.0f, 1.0f)) };
}
