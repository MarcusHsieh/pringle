#include "DroneApp.hpp"
#include <iostream>
#include <fstream>
#include <chrono>

using namespace std::chrono;
using namespace std::chrono_literals;

DroneApp::DroneApp() = default;

bool DroneApp::init() {
    if (!motors_.init()) { std::cerr << "[Drone] Motors init failed\n"; return false; }
    if (!servo_.init())  { std::cerr << "[Drone] Servo init failed\n";  return false; }

    if (!camera_.init()) {
        std::cerr << "[Drone] Camera init failed — FPV disabled\n";
    } else {
        flags_ |= Flags::CAMERA_OK;
    }

    if (!radio_.init()) { std::cerr << "[Drone] Radio init failed\n"; return false; }

    motors_.arm();
    flags_ |= Flags::MOTORS_ARMED;

    radio_.queueTelemetry(buildTelemetry());

    running_ = true;
    std::cout << "[Drone] Ready\n";
    return true;
}

void DroneApp::stop() {
    running_ = false;
}

void DroneApp::run() {
    std::thread radioThread([this]{ radioLoop(); });
    radioThread.join();

    motors_.neutral();
    servo_.center();
    camera_.stop();
    std::cout << "[Drone] Shutdown complete\n";
}

void DroneApp::radioLoop() {
    // Safe defaults: neutral on all channels
    ControlPacket ctrl{ 7.5f, 7.5f, 7.5f };

    auto lastPacket = steady_clock::now();

    while (running_) {
        if (radio_.receive(ctrl)) {
            lastPacket = steady_clock::now();

            // Duty% flows straight through — no conversion anywhere
            motors_.setLeft(ctrl.leftDuty);
            motors_.setRight(ctrl.rightDuty);
            servo_.set(ctrl.servoDuty);

            ++telemSeq_;
            radio_.queueTelemetry(buildTelemetry());
        }

        // Safety: link lost → neutral
        const auto silenceMs = duration_cast<milliseconds>(
            steady_clock::now() - lastPacket).count();

        if (silenceMs > LINK_TIMEOUT_MS) {
            motors_.neutral();
            servo_.center();

            static auto lastWarn = steady_clock::now() - 2s;
            if (steady_clock::now() - lastWarn > 1s) {
                std::cerr << "[Drone] Link timeout — motors neutral\n";
                lastWarn = steady_clock::now();
            }
        }

        std::this_thread::sleep_for(1ms);
    }
}

TelemetryPacket DroneApp::buildTelemetry() const {
    return TelemetryPacket{
        .seq     = telemSeq_,
        .cpuTemp = readCpuTemp(),
        .flags   = flags_
    };
}

uint8_t DroneApp::readCpuTemp() const {
    std::ifstream f("/sys/class/thermal/thermal_zone0/temp");
    int milliC = 0;
    if (f >> milliC) return static_cast<uint8_t>(milliC / 1000);
    return 0;
}
