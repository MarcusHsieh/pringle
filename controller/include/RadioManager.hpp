#pragma once
#include <RF24/RF24.h>
#include "shared/protocol.hpp"

// Controller-side radio: PTX (primary transmitter).
// Sends ControlPacket to the drone.
// Drone replies with TelemetryPacket piggybacked on the ACK payload —
// no role swap, no extra round-trip.
class RadioManager {
public:
    RadioManager();
    bool init();

    // Transmits pkt. Returns true if the drone acknowledged.
    // If ACK carried telemetry, fills *telem (pass nullptr to ignore).
    bool sendControl(const ControlPacket& pkt, TelemetryPacket* telem = nullptr);

private:
    RF24 radio_;

    static constexpr uint8_t CE_PIN  = 25;  // GPIO25 → nRF24L01 CE
    static constexpr uint8_t CSN_PIN = 0;   // SPI0 CE0 → nRF24L01 CSN
};
