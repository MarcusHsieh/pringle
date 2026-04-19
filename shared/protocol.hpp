#pragma once
#include <cstdint>

// RF24 settings
// Controller -> PTX (Primary Transmitter)
// Drone -> PRX (Primary Receiver)
// Basically, whenever controller sends packet, drone acks it back w/ its own packet
constexpr uint8_t RF_CHANNEL        = 76;
constexpr uint8_t CTRL_ADDR[6]      = "1CTRL";   // controller to drone

// Control packet (12 bytes)
// Pulse widths in duty cycle percent @ 50hz -> motor + servo
//    5.0 = full reverse (1.0ms pulse)
//    7.5 = stop/neutral (1.5ms pulse)
//   10.0 = full forward (2.0ms pulse)
struct __attribute__((packed)) ControlPacket {
    float leftDuty;    // left motor 
    float rightDuty;   // right motor
    float servoDuty;   // camera gimbal pitch
};

// Telemetry packet (3 bytes)
struct __attribute__((packed)) TelemetryPacket {
    uint8_t seq;            // rolling counter (0-255) for packet loss detection, ensures link is alive
    uint8_t cpuTemp;        // drone CPU temperature in Celsius
    uint8_t flags;          // bitmask
};

namespace Flags {
    constexpr uint8_t MOTORS_ARMED = 1u << 0;
    constexpr uint8_t CAMERA_OK    = 1u << 1;
}