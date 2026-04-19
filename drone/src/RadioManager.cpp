#include "RadioManager.hpp"
#include <iostream>

RadioManager::RadioManager() : radio_(CE_PIN, CSN_PIN) {}

bool RadioManager::init() {
    if (!radio_.begin()) {
        std::cerr << "[Radio] Hardware not responding\n";
        return false;
    }

    radio_.setChannel(RF_CHANNEL);
    radio_.setPALevel(RF24_PA_LOW);
    radio_.setDataRate(RF24_250KBPS);

    radio_.enableDynamicPayloads();
    radio_.enableAckPayload();

    radio_.openReadingPipe(1, CTRL_ADDR);       // pipe 1 = where control packets arrive

    radio_.flush_rx();
    radio_.flush_tx();

    radio_.startListening();                    // PRX mode

    radio_.printDetails();
    std::cout << "[Radio] Drone radio OK - listening on channel " << (int)RF_CHANNEL << "\n";
    radio_.printDetails();
    return true;
}

bool RadioManager::receive(ControlPacket& pkt) {
    if (!radio_.available()) {
        std::cout << "[Radio] No packet available\n";
        return false;
    }
    uint8_t size = radio_.getDynamicPayloadSize();
    if (size > 32) {
        std::cout << "[Radio] Invalid packet size: " << (int)size << "\n";
        radio_.flush_rx();
        return false;
    }
    radio_.read(&pkt, size);
    std::cout << "[Radio] Received packet" << "\n";
    return true;
}

void RadioManager::queueTelemetry(const TelemetryPacket& telem) {
    // Pipe 1 == reading pipe opened in init()
    radio_.writeAckPayload(1, &telem, sizeof(telem));
}
