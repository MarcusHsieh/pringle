#pragma once
#include <cstdint>

// ── RF config ────────────────────────────────────────────────────────────────
constexpr uint8_t RF_CHANNEL   = 76;
constexpr uint8_t CTRL_ADDR[6] = "1CTRL";

// ── Controller → Drone ──────────────────────────────────────────────────────
// All motor/servo values are duty cycle percent at 50 Hz.
//   5.0 = full reverse   (1.0 ms pulse)
//   7.5 = neutral / stop (1.5 ms pulse)  ← ESC arming point
//  10.0 = full forward   (2.0 ms pulse)
struct ControlPacket {
    float leftDuty;   // 5.0 – 10.0
    float rightDuty;
    float servoDuty;
};  // 12 bytes — well within the 32-byte nRF24L01 payload limit

// ── Drone → Controller (ACK payload) ────────────────────────────────────────
struct TelemetryPacket {
    uint8_t seq;      // rolling 0–255: proves link alive, detects drops
    uint8_t cpuTemp;  // °C from /sys/class/thermal/thermal_zone0/temp
    uint8_t flags;    // bitmask — see Flags::
};  // 3 bytes

namespace Flags {
    constexpr uint8_t MOTORS_ARMED = 1u << 0;
    constexpr uint8_t CAMERA_OK    = 1u << 1;
}
