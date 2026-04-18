#include "workers.hpp"

// 0 = Neutral/Armed, 1 = High (Forward), 2 = Low (Reverse/Brake)
std::atomic<int> motor_state(1);

void motor_worker() {
    int esc_pin = 18; // Physical Pin 12
    if (lgGpioClaimOutput(h, 0, esc_pin, 0) < 0) return;

    // --- STEP 1: THE HANDSHAKE (ARMING) ---
    // We MUST hold 7.5% steady so the ESC hears the "Neutral" it wants.
    std::cout << "[Motor] Sending Neutral (7.5%). PLUG IN BATTERY NOW..." << std::endl;
    
    for(int i = 0; i < 40; i++) { // 4 seconds total
        lgTxPwm(h, esc_pin, 50, 7.5, 0, 0); 
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        if(i == 20) std::cout << "[Motor] Still waiting for ESC to arm..." << std::endl;
    }

    // You should hear the "second beep" during the 4 seconds above.
    std::cout << "[Motor] ESC should be armed. Starting ramp-up!" << std::endl;

    // --- STEP 2: THE RAMP ---
    double current_throttle = 7.5;
    while(running) {
        if (current_throttle < 10.0) {
            current_throttle += 0.05;
        }
        else {
            current_throttle = 7.5; // Loop back to Neutral
        }

        std::cout << "Throttle: " << current_throttle << "%" << std::endl;
        lgTxPwm(h, esc_pin, 50, current_throttle, 0, 0); 
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    lgTxPwm(h, esc_pin, 50, 7.5, 0, 0); // Safety Neutral
}

void camera_worker() {
    // 1. Open the Frame Buffer
    int fb_fd = open("/dev/fb0", O_RDWR);
    if (fb_fd == -1) {
        std::cerr << "[Camera] Error: Cannot open /dev/fb0 even though it exists!" << std::endl;
        return;
    }

    // 2. Open USB Camera
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "[Camera] Error: USB Camera not detected." << std::endl;
        close(fb_fd);
        return;
    }

    // Match the 640x480 resolution we set in config.txt
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    cv::Mat frame, bgra_frame;
    std::cout << "[Camera] SUCCESS: Streaming to 3.5mm Radio Jack..." << std::endl;

    while (running) {
        std::cout << "Camera Worker Running" << std::endl;
        cap >> frame;
        if (frame.empty()) continue;

        // Convert 3-channel BGR to 4-channel BGRA (Standard for Pi Framebuffer)
        cv::cvtColor(frame, bgra_frame, cv::COLOR_BGR2BGRA);

        // Write directly to the hardware memory
        lseek(fb_fd, 0, SEEK_SET);
        write(fb_fd, bgra_frame.data, bgra_frame.total() * bgra_frame.elemSize());

        // Sync with the radio's refresh rate (~30fps)
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }

    cap.release();
    close(fb_fd);
}


void listen_worker() {
    std::cout << "[Listen] Press 'k' + Enter to toggle HIGH/LOW" << std::endl;
    char input;
    while(running) {
        std::cin >> input;
        if (input == 'k') {
            if (motor_state == 1) {
                motor_state = 2;
                std::cout << ">> Setting PWM to LOW (5%)" << std::endl;
            } else {
                motor_state = 1;
                std::cout << ">> Setting PWM to HIGH (10%)" << std::endl;
            }
        }
    }
}