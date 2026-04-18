#include "workers.hpp"

// 0 = Neutral/Armed, 1 = High (Forward), 2 = Low (Reverse/Brake)
std::atomic<int> motor_state(1);


MPU6050 imu = MPU6050(0x68, true); // I2C address, bool run update thread

void motor_worker() {
    int esc_pin = 18;
    if (lgGpioClaimOutput(h, 0, esc_pin, 0) < 0) return;

    // variables for IMU data
    float accel_x, accel_y, accel_z, gyro_roll, gyro_pitch, gyro_yaw;


    std::cout << "[Motor] Sending Neutral (7.5%). PLUG IN BATTERY NOW..." << std::endl;
    for(int i = 0; i < 40; i++) { // 4 seconds total
        lgTxPwm(h, esc_pin, 50, 7.5, 0, 0); 
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        if(i == 20) std::cout << "[Motor] Still waiting for ESC to arm..." << std::endl;
    }

    std::cout << "[Motor] ESC should be armed. Starting ramp-up!" << std::endl;
    double current_throttle = 7.5;
    while(running) {
        std::cout << "Reading IMU Data..." << std::endl;
        imu.getAccel(&accel_x, &accel_y, &accel_z);
        imu.getGyro(&gyro_roll, &gyro_pitch, &gyro_yaw);
        std::cout << "Accel (g): X=" << accel_x << " Y=" << accel_y << " Z=" << accel_z << std::endl;
        std::cout << "Gyro (°/s): Roll=" << gyro_roll << " Pitch=" << gyro_pitch << " Yaw=" << gyro_yaw << std::endl;
        
        std::cout << "Motor Worker Running. Current Throttle: " << current_throttle << "%" << std::endl;
        if (current_throttle < 10.0) {
            current_throttle += 0.05;
        }
        else {
            current_throttle = 7.5; // Loop back to Neutral
        }
        lgTxPwm(h, esc_pin, 50, current_throttle, 0, 0); 
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    lgTxPwm(h, esc_pin, 50, 7.5, 0, 0);
}

void camera_worker() {
    int fb_fd = open("/dev/fb0", O_RDWR);
    if (fb_fd == -1) {
        std::cerr << "[Camera] Error: Cannot open /dev/fb0 even though it exists!" << std::endl;
        return;
    }

    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "[Camera] Error: USB Camera not detected." << std::endl;
        close(fb_fd);
        return;
    }

    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    cv::Mat frame, bgra_frame;
    std::cout << "[Camera] SUCCESS: Streaming to 3.5mm Radio Jack..." << std::endl;

    while (running) {
        std::cout << "Camera Worker Running" << std::endl;
        cap >> frame;
        if (frame.empty()) continue;

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
    RadioManager radio = RadioManager();
    ControlPacket pkt = ControlPacket();
    TelemetryPacket telem = TelemetryPacket();
    if (!radio.init()) {
        std::cout << "[Listen Worker] Failed to initialize radio. Exiting listen worker thread." << std::endl;
        return;
    }
    else {
        std::cout << "[Listen Worker] Radio initialized successfully." << std::endl;
    }

    while(running) {
        std::cout << "Listen Worker Running." << std::endl;
        radio.receive(pkt);
        radio.queueTelemetry(telem);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return;
}