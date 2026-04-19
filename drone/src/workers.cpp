#include "workers.hpp"

// Shared atomic setpoints — gamepad_worker writes, motor_worker reads
std::atomic<float> raw_fwd{0.0f};
std::atomic<float> raw_turn{0.0f};
std::atomic<float> raw_servo{0.0f};

static constexpr float THROTTLE_CAP = 0.65f;  // limit to 65% of [5.0, 10.0] range

// Maps normalized [-1, 1] to duty cycle percent [5.0, 10.0]
// neutral (0.0) → 7.5%, full forward (1.0) → 10.0%, full reverse (-1.0) → 5.0%
static inline float toDuty(float v) {
    return std::clamp(7.5f + v * 2.5f, 5.0f, 10.0f);
}

// ─── gamepad_worker ──────────────────────────────────────────────────────────
// Reads USB gamepad dongle at ~200 Hz.
// Mixes right-stick fwd/turn, d-pad servo into normalized atomics.
void gamepad_worker() {
    GamepadReader gamepad;
    if (!gamepad.init()) {
        std::cerr << "[Gamepad] Init failed — will retry when device appears\n";
    }

    GamepadState state{};
    while (running) {
        gamepad.update(state);

        // Apply throttle cap and write atomics
        raw_fwd.store(state.rightY  * THROTTLE_CAP);
        raw_turn.store(state.rightX * THROTTLE_CAP);
        raw_servo.store(state.servoPos);

        std::this_thread::sleep_for(std::chrono::milliseconds(5));  // ~200 Hz
    }
}

// ─── motor_worker ─────────────────────────────────────────────────────────────
// 50 Hz control loop:
//   1. Rate-limit raw gamepad setpoints
//   2. Read complementary-filtered pitch from MPU6050
//   3. PID correction added symmetrically to both motors (preserves steering mix)
//   4. Output PWM via lgpio
void motor_worker() {
    // Claim GPIO pins
    if (lgGpioClaimOutput(h, 0, MOTOR_PIN_RIGHT, 0) < 0 ||
        lgGpioClaimOutput(h, 0, MOTOR_PIN_LEFT,  0) < 0 ||
        lgGpioClaimOutput(h, 0, SERVO_PIN,       0) < 0) {
        std::cerr << "[Motor] GPIO claim failed — check pin assignments\n";
        return;
    }

    // ESC arming sequence — neutral for 4 seconds
    std::cout << "[Motor] Sending neutral (7.5%). Plug in battery now...\n";
    for (int i = 0; i < 200; ++i) {  // 200 × 20ms = 4s at 50 Hz
        lgTxPwm(h, MOTOR_PIN_RIGHT, 50, 7.5, 0, 0);
        lgTxPwm(h, MOTOR_PIN_LEFT,  50, 7.5, 0, 0);
        lgTxPwm(h, SERVO_PIN,       50, 7.5, 0, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        if (i == 100) std::cout << "[Motor] Still arming ESCs...\n";
    }
    std::cout << "[Motor] ESCs armed — starting control loop\n";

    // IMU — background thread updates complementary filter continuously
    MPU6050 imu(0x68, true);
    imu.calc_yaw = false;

    PIDController    pid;
    StabilizationState stab;

    struct timespec prev_ts, curr_ts;
    clock_gettime(CLOCK_MONOTONIC, &prev_ts);

    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));  // 50 Hz

        // Compute dt in seconds
        clock_gettime(CLOCK_MONOTONIC, &curr_ts);
        const float dt = static_cast<float>(curr_ts.tv_sec  - prev_ts.tv_sec) +
                         static_cast<float>(curr_ts.tv_nsec - prev_ts.tv_nsec) * 1e-9f;
        prev_ts = curr_ts;

        // Rate-limit gamepad inputs
        stab.update(raw_fwd.load(), raw_turn.load(), dt);

        // Pitch angle from complementary filter (degrees, + = nose up)
        float pitch = 0.0f;
        imu.getAngle(1, &pitch);

        // PID: error = 0° - current_pitch; output added to both motors equally
        const float correction = pid.calculate(0.0f, pitch, dt);

        // Differential drive mix + pitch correction
        const float fwd_corrected = stab.ramped_fwd + correction;
        const float left_duty  = toDuty(fwd_corrected + stab.ramped_turn);
        const float right_duty = toDuty(fwd_corrected - stab.ramped_turn);
        const float servo_duty = toDuty(raw_servo.load());

        lgTxPwm(h, MOTOR_PIN_LEFT,  50, left_duty,  0, 0);
        lgTxPwm(h, MOTOR_PIN_RIGHT, 50, right_duty, 0, 0);
        lgTxPwm(h, SERVO_PIN,       50, servo_duty, 0, 0);
    }

    // Safe stop
    lgTxPwm(h, MOTOR_PIN_RIGHT, 50, 7.5, 0, 0);
    lgTxPwm(h, MOTOR_PIN_LEFT,  50, 7.5, 0, 0);
    lgTxPwm(h, SERVO_PIN,       50, 7.5, 0, 0);
}

// ─── camera_worker ───────────────────────────────────────────────────────────
// Captures USB camera frames and writes them directly to /dev/fb0 (framebuffer).
// The framebuffer feeds the TS832 5.8 GHz FPV transmitter via composite output.
void camera_worker() {
    int fb_fd = open("/dev/fb0", O_RDWR);
    if (fb_fd == -1) {
        std::cerr << "[Camera] Cannot open /dev/fb0\n";
        return;
    }

    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "[Camera] USB camera not detected\n";
        close(fb_fd);
        return;
    }

    cap.set(cv::CAP_PROP_FRAME_WIDTH,  CAMERA_WIDTH);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, CAMERA_HEIGHT);

    std::cout << "[Camera] Streaming to framebuffer → TS832 FPV TX\n";

    cv::Mat frame, bgra_frame;
    while (running) {
        cap >> frame;
        if (frame.empty()) continue;

        cv::cvtColor(frame, bgra_frame, cv::COLOR_BGR2BGRA);
        lseek(fb_fd, 0, SEEK_SET);
        write(fb_fd, bgra_frame.data, bgra_frame.total() * bgra_frame.elemSize());

        std::this_thread::sleep_for(std::chrono::milliseconds(30));  // ~30 fps
    }

    cap.release();
    close(fb_fd);
}
