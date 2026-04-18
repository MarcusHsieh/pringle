#include "Controller.hpp"
#include <iostream>
#include <csignal>

static Controller* gController = nullptr;

static void handleSignal(int) {
    if (gController) gController->stop();
}

int main() {
    std::signal(SIGINT,  handleSignal);
    std::signal(SIGTERM, handleSignal);

    Controller ctrl;
    gController = &ctrl;

    if (!ctrl.init()) {
        std::cerr << "Controller init failed";
        return 1;
    }

    ctrl.run();   // blocks until stop() is called
    return 0;
}
