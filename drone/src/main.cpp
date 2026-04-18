#include "main.hpp"
#include "workers.hpp"

// gpio controller chip handle
int h;
bool running = true;

int main() {
    // initialize gpio
    h = lgGpiochipOpen(0);

    std::cout << "Booting up Threads" << std::endl;
    
    // create threads
    std::thread motor_thread(motor_worker);
    std::thread camera_thread(camera_worker);
    std::thread listen_thread(listen_worker);

    // join threads
    motor_thread.join();
    camera_thread.join();
    listen_thread.join();

    lgGpiochipClose(h);
    running = false;
    return 0;
}