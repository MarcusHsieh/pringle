#include "RadioManager.hpp"
#include <iostream>

RadioManager::RadioManager() : radio_(CE_PIN, CSN_PIN) {}

bool RadioManager::init() {
    if (!radio_.begin(CE_PIN, CSN_PIN)) {
        std::cerr << "[Radio] Hardware not responding\n";
        return false;
    }

    radio_.setChannel(RF_CHANNEL);
    radio_.setPALevel(RF24_PA_LOW);
    radio_.setDataRate(RF24_250KBPS);

    radio_.disableDynamicPayloads();
    radio_.setAutoAck(false);
    radio_.setPayloadSize(sizeof(ControlPacket)); // fixed payload size for simplicity

    radio_.openReadingPipe(1, CTRL_ADDR);       // pipe 1 = where control packets arrive

    radio_.flush_rx();
    radio_.flush_tx();

    radio_.startListening();                    // PRX mode
    radio_.ce(true);

    radio_.printDetails();
    std::cout << "[Radio] Drone radio OK - listening on channel " << (int)RF_CHANNEL << "\n";
    radio_.printPrettyDetails();
    return true;
}

bool RadioManager::receive(ControlPacket& pkt) {
    if (!radio_.available()) {
        // if (radio_.testRPD()) {
        //     std::cout << "[Radio Warning] Massive interference detected on Channel " << (int)RF_CHANNEL << "!\n";
        //     return false;
        // }
        // else {
        //     std::cout << "[Radio] No packet available\n";
        // }
        return false;
    }

    if (radio_.failureDetected) {
        std::cerr << "[Radio] Hardware lockup detected! Rebooting radio..." << std::endl;
        radio_.begin(); // Re-run your setup!
        radio_.failureDetected = false; 
    }


    radio_.read(&pkt, sizeof(pkt));
    std::cout << "[Radio] Received packet" << "\n";
    // radio_.printPrettyDetails();
    return true;
}

void RadioManager::queueTelemetry(const TelemetryPacket& telem) {
    // Pipe 1 == reading pipe opened in init()
    radio_.writeAckPayload(1, &telem, sizeof(telem));
}
