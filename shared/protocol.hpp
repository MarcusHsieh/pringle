#pragma once
#include <cstdint>

// RF24 settings
// Controller -> PTX (Primary Transmitter)
// Drone -> PRX (Primary Receiver)
// Basically, whenever controller sends packet, drone acks it back w/ its own packet
constexpr uint8_t RF_CHANNEL        = 76;
constexpr uint8_t CTRL_ADDR[6]      = "1CTRL";   // controller to drone

// Control packet (6 bytes)
// Pulse widths in microseconds
struct ControlPacket {
    int16_t leftPW;    // left motor (1000=full reverse, 1500=stop, 2000=full forward)
    int16_t rightPW;   // right motor (same as above)
    int16_t servoPW;   // camera gimbal pitch (1500=center)
};

// Telemetry packet (3 bytes)
struct TelemetryPacket {
    uint8_t seq;            // rolling counter (0-255) for packet loss detection, ensures link is alive
    uint8_t cpuTemp;        // drone CPU temperature in Celsius
    uint8_t flags;          // bitmask
};

namespace Flags {
    constexpr uint8_t MOTORS_ARMED = 1u << 0;
    constexpr uint8_t CAMERA_OK    = 1u << 1;
}