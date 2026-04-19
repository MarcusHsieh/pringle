#include "Controller.hpp"
#include <iostream>
#include <csignal>
#include <cstdlib>

static Controller* gController = nullptr;

static void handleSignal(int) {
    if (gController) gController->stop();
}

int main() {
    setenv("QT_QPA_PLATFORM", "xcb", 0);

    std::signal(SIGINT,  handleSignal);
    std::signal(SIGTERM, handleSignal);

    Controller ctrl;
    gController = &ctrl;

    if (!ctrl.init()) {
        std::cerr << " >> Controller init failed << \n";
        return 1;
    }

    ctrl.run();   // blocks until stop() is called
    return 0;
}
