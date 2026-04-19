#pragma once
#include <lgpio.h>

// Controls the two Readytosky 50A bidirectional ESCs via lgTxPwm.
//
// Duty cycle at 50 Hz — the one true unit for this project:
//   5.0% = full reverse   (1.0 ms pulse)
//   7.5% = neutral / stop (1.5 ms pulse)  ← ESC arming point
//  10.0% = full forward   (2.0 ms pulse)
//
// Wiring:
//   GPIO18 (pin 12) → left ESC signal  (white/yellow)
//   GPIO13 (pin 33) → right ESC signal
//   Any GND pin     → ESC signal ground (brown/black)
//
// NOTE: lgpio uses BCM GPIO numbers, NOT physical pin numbers.
class MotorController {
public:
    MotorController();
    ~MotorController();

    bool init();

    void setLeft(float duty);    // 5.0 – 10.0
    void setRight(float duty);
    void neutral();              // both → 7.5%
    void arm();                  // holds neutral 4 s for ESC arming sequence

private:
    static float clamp(float duty);

    int lgh_ = -1;

    static constexpr int   GPIO_LEFT  = 18;
    static constexpr int   GPIO_RIGHT = 13;
    static constexpr float FREQ_HZ    = 50.0f;
    static constexpr float DUTY_MIN   = 5.0f;
    static constexpr float DUTY_MAX   = 10.0f;
    static constexpr float DUTY_NEUTRAL = 7.5f;
};
