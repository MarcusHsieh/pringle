#pragma once
#include <cstdint>

// RF24 settings
constexpr uint8_t RF_CHANNEL        = 76;
constexpr uint8_t CTRL_ADDR[6]      = "1CTRL";   // controller to drone
constexpr uint8_t TELEM_ADDR[6]     = "1TELM";   // drone to controller

// Control packet: controller transmits, drone receives
// All pulse widths in microseconds (1000=full reverse, 1500=stop, 2000=full forward)
struct ControlPacket {
    int16_t leftPW;    // left motor
    int16_t rightPW;   // right motor
    int16_t servoPW;   // camera gimbal pitch
};
// 6 bytes total

// Telemetry packet: drone transmits back (stub for now)
struct TelemetryPacket {
    uint8_t seq;   // rolling counter for packet loss detection
    uint8_t  cpuTemp;       // drone CPU temperature in Celsius
};
// 2 bytes total