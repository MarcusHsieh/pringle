#include "main.hpp"
#include "workers.hpp"

int h;
bool running = true;

int main() {
    h = lgGpiochipOpen(0);

    std::cout << "[Drone] Starting threads\n";

    std::thread gamepad_thread(gamepad_worker);
    std::thread motor_thread(motor_worker);
    std::thread camera_thread(camera_worker);

    motor_thread.join();
    camera_thread.join();
    gamepad_thread.join();

    lgGpiochipClose(h);
    running = false;
    return 0;
}
