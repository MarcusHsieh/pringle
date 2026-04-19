#include "RadioManager.hpp"
#include <iostream>

RadioManager::RadioManager() : radio_(CE_PIN, CSN_PIN) {}

bool RadioManager::init() {
    if (!radio_.begin()) {
        std::cerr << "[Radio] Hardware not responding\n";
        return false;
    }

    radio_.setChannel(RF_CHANNEL);
    radio_.setPALevel(RF24_PA_LOW);      // max power; drop to RF24_PA_LOW for bench testing
    radio_.setDataRate(RF24_250KBPS);     // lowest data rate = best range and noise rejection
    radio_.setRetries(5, 15);             // retry up to 15x with 5x250µs = 1.25ms b/n tries
    radio_.enableDynamicPayloads();
    radio_.enableAckPayload();            // telemetry rides on the ACK, no role swap

    radio_.openWritingPipe(CTRL_ADDR);
    radio_.flush_tx();

    radio_.stopListening();               // PTX mode

    radio_.printDetails();
    std::cout << "[Radio] Controller radio OK - channel " << (int)RF_CHANNEL << "\n";
    return true;
}

bool RadioManager::sendControl(const ControlPacket& pkt, TelemetryPacket* telem) {
    const bool acked = radio_.write(&pkt, sizeof(pkt));
    std::printf("Control packet sent: %s\n", acked ? "true" : "false");

    if (acked && telem && radio_.isAckPayloadAvailable()) {
        uint8_t size = radio_.getDynamicPayloadSize();
        if (size > 0 && size <= 32) {
            radio_.read(telem, size);
        }
        else {
            radio_.flush_rx();
        }
    }
    return acked;
}
