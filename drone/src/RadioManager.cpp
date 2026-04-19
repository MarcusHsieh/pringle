#include "RadioManager.hpp"
#include <iostream>

RadioManager::RadioManager() : radio_(CE_PIN, CSN_PIN) {}

bool RadioManager::init() {
    if (!radio_.begin()) {
        std::cerr << "[Radio] Hardware not responding — check SPI wiring and 3.3V power\n";
        return false;
    }

    radio_.setChannel(RF_CHANNEL);
    radio_.setPALevel(RF24_PA_HIGH);
    radio_.setDataRate(RF24_250KBPS);
    radio_.setRetries(5, 15);
    radio_.enableAckPayload();                  // allows writeAckPayload()
    radio_.setPayloadSize(sizeof(ControlPacket));
    radio_.openReadingPipe(1, CTRL_ADDR);       // pipe 1 = where control packets arrive
    radio_.startListening();                    // PRX mode

    std::cout << "[Radio] Drone radio OK — listening on channel " << (int)RF_CHANNEL << "\n";
    return true;
}

bool RadioManager::receive(ControlPacket& pkt) {
    if (!radio_.available()) return false;
    radio_.read(&pkt, sizeof(pkt));
    return true;
}

void RadioManager::queueTelemetry(const TelemetryPacket& telem) {
    // Pipe 1 must match the reading pipe opened in init()
    radio_.writeAckPayload(1, &telem, sizeof(telem));
}
