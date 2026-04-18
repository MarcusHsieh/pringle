#pragma once
#include <RF24/RF24.h>
#include "shared/protocol.hpp"

// Drone-side radio: PRX (primary receiver)
// Listens for ControlPacket from the controller
// Queues TelemetryPacket to ride back on the ACK
//  for controller to read it after each successful write
class RadioManager {
public:
    RadioManager();
    bool init();

    // Non-blocking, returns true and fills pkt if a new packet arrived
    bool receive(ControlPacket& pkt);

    // Loads telem as the ACK payload for the next incoming packet
    // Call this after every receive() so telemetry stays fresh
    void queueTelemetry(const TelemetryPacket& telem);

private:
    RF24 radio_;

    static constexpr uint8_t CE_PIN  = 25;  // GPIO25 -> nRF24L01 CE
    static constexpr uint8_t CSN_PIN = 0;   // SPI0 CE0 -> nRF24L01 CSN
};
