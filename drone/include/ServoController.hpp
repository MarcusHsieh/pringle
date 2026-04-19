#pragma once
#include <lgpio.h>

// Controls the camera tilt servo via lgTxPwm.
// Same duty cycle unit as motors: 7.5% = center, 5.0% = one extreme, 10.0% = other.
//
// GPIO19 (physical pin 35) — GPIO18 is taken by left motor.
//
// Wiring:
//   GPIO19 (pin 35) → servo signal  (orange/yellow)
//   LV 5V rail      → servo power   (red)
//   LV GND          → servo ground  (brown/black)
class ServoController {
public:
    ServoController();
    ~ServoController();

    bool init();
    void set(float duty);   // 5.0 – 10.0
    void center();          // 7.5%

private:
    int lgh_ = -1;

    static constexpr int   GPIO_SERVO    = 19;
    static constexpr float FREQ_HZ       = 50.0f;
    static constexpr float DUTY_MIN      = 5.0f;
    static constexpr float DUTY_MAX      = 10.0f;
    static constexpr float DUTY_NEUTRAL  = 7.5f;
};
