#pragma once
#include <thread>
#include <atomic>
#include <chrono>
#include "RadioManager.hpp"
#include "MotorController.hpp"
#include "ServoController.hpp"
#include "CameraCapture.hpp"
#include "shared/protocol.hpp"

class DroneApp {
public:
    DroneApp();
    bool init();
    void run();    // launches threads, blocks until stop()
    void stop();

private:
    // ── Thread loops ────────────────────────────────────────────────────────
    // radioLoop receives ControlPackets and drives motors/servo directly.
    // cameraLoop is owned by CameraCapture (already threaded internally).
    void radioLoop();

    // ── Telemetry helpers ───────────────────────────────────────────────────
    TelemetryPacket buildTelemetry() const;
    uint8_t         readCpuTemp()    const;

    // ── Hardware ─────────────────────────────────────────────────────────────
    RadioManager    radio_;
    MotorController motors_;
    ServoController servo_;
    CameraCapture   camera_;

    // ── State ────────────────────────────────────────────────────────────────
    std::atomic<bool>  running_{false};
    uint8_t            telemSeq_ = 0;
    uint8_t            flags_    = 0;

    // Safety: if no packet arrives within this window, go neutral
    static constexpr int LINK_TIMEOUT_MS = 500;
};
