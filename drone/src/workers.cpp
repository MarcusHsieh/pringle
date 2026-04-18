#include "workers.hpp"

std::mutex command_mutex; // ensures thread safe access to command variables
MPU6050 imu = MPU6050(0x68, true); // I2C address, bool run update thread

// variables for motor commands
float left_command = 7.5f;
float right_command = 7.5f;
float servo_command = 7.5f;

// variables for IMU data
float accel_x = 0.0f;
float accel_y = 0.0f; 
float accel_z = 0.0f; 
float gyro_roll = 0.0f; 
float gyro_pitch = 0.0f; 
float gyro_yaw = 0.0f;

void motor_worker() {
    PIDController pid;
    pid.Kp = 0.4f;
    pid.Ki = 0.01f;
    pid.Kd = 0.1f;
    float dt = 0.1f; // 100ms loop
    float correction = 0.0f;

    if (lgGpioClaimOutput(h, 0, MOTOR_PIN_RIGHT, 0) < 0 || lgGpioClaimOutput(h, 0, MOTOR_PIN_LEFT, 0) < 0 || lgGpioClaimOutput(h, 0, SERVO_PIN, 0) < 0) {
        std::cout << "Motor Initialization Failure. Check GPIO Pins." << std::endl;
        return;
    }

    std::cout << "[Motor] Sending Neutral (7.5%). PLUG IN BATTERY NOW..." << std::endl;
    for(int i = 0; i < 40; i++) { // 4 seconds total
        lgTxPwm(h, MOTOR_PIN_RIGHT, 50, 7.5, 0, 0);
        lgTxPwm(h, MOTOR_PIN_LEFT, 50, 7.5, 0, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        if(i == 20) std::cout << "[Motor] Still waiting for ESC to arm..." << std::endl;
    }

    while(running) {
        imu.getAccel(&accel_x, &accel_y, &accel_z);
        imu.getGyro(&gyro_roll, &gyro_pitch, &gyro_yaw);

        if (std::abs(gyro_pitch) > 10.0f) {
            correction = pid.calculate(0.0f, gyro_pitch, dt);
        }
        else {
            pid.integral = 0.0f;
        }
        
        // use command variables to control motor speeds
        {
            std::lock_guard<std::mutex> lock(command_mutex);
            std::cout << "Current IMU Readings - Gyro (Roll, Pitch, Yaw): (" << gyro_roll << ", " << gyro_pitch << ", " << gyro_yaw << ") deg/s" << std::endl;

            right_command = right_command + correction;
            left_command = left_command - correction;

            lgTxPwm(h, MOTOR_PIN_RIGHT, 50, right_command, 0, 0);
            lgTxPwm(h, MOTOR_PIN_LEFT, 50, left_command, 0, 0);
            lgTxPwm(h, SERVO_PIN, 50, servo_command, 0, 0);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // set motors to neutral on exit
    lgTxPwm(h, MOTOR_PIN_RIGHT, 50, 7.5, 0, 0);
    lgTxPwm(h, MOTOR_PIN_LEFT, 50, 7.5, 0, 0);
    lgTxPwm(h, SERVO_PIN, 50, 7.5, 0, 0);
    return;
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

        // digest packet into variables for control
        {
            std::lock_guard<std::mutex> lock(command_mutex);
            left_command = pkt.leftDuty;
            right_command = pkt.rightDuty;
            servo_command = pkt.servoDuty;
            std::cout << "Received Control Packet - Left Duty: " << left_command << "%, Right Duty: " << right_command << "%, Servo Duty: " << servo_command << "%" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return;
}