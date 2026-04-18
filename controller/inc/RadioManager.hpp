#pragma once
#include <RF24/RF24.h>
#include "shared/protocol.hpp"

// Controller-side radio: PTX (primary transmitter)
// Sends ControlPacket to drone
// Drone replies w/ TelemetryPacket piggybacked on the ACK payload
class RadioManager {
public:
    RadioManager();
    bool init();

    // Transmits pkt, returns true if the drone acked it
    //   if ACK carried telemetry, fills *telem
    bool sendControl(const ControlPacket& pkt, TelemetryPacket* telem = nullptr);

private:
    RF24 radio_;

    static constexpr uint8_t CE_PIN  = 25;  // GPIO25 -> nRF24L01 CE
    static constexpr uint8_t CSN_PIN = 0;   // SPI0 CE0 -> nRF24L01 CSN
};
