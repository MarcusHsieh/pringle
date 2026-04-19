#include "RadioManager.hpp"
#include <iostream>

RadioManager::RadioManager() : radio_(CE_PIN, CSN_PIN) {}

bool RadioManager::init() {
    if (!radio_.begin()) {
        std::cerr << "[Radio] Hardware not responding — check SPI wiring and 3.3V power\n";
        return false;
    }

    radio_.setChannel(RF_CHANNEL);
    radio_.setPALevel(RF24_PA_HIGH);      // max power; drop to RF24_PA_LOW for bench testing
    radio_.setDataRate(RF24_250KBPS);     // lowest data rate = best range and noise rejection
    radio_.setRetries(5, 15);             // retry up to 15× with 5×250µs = 1.25ms between tries
    radio_.enableAckPayload();            // telemetry rides on the ACK, no role swap needed
    radio_.setPayloadSize(sizeof(ControlPacket));
    radio_.openWritingPipe(CTRL_ADDR);
    radio_.stopListening();               // PTX mode

    std::cout << "[Radio] Controller radio OK — channel " << (int)RF_CHANNEL << "\n";
    return true;
}

bool RadioManager::sendControl(const ControlPacket& pkt, TelemetryPacket* telem) {
    const bool acked = radio_.write(&pkt, sizeof(pkt));

    if (acked && telem && radio_.isAckPayloadAvailable()) {
        radio_.read(telem, sizeof(TelemetryPacket));
    }

    return acked;
}
