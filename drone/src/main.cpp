#include "DroneApp.hpp"
#include <iostream>
#include <csignal>

static DroneApp* gApp = nullptr;

static void handleSignal(int) {
    if (gApp) gApp->stop();
}

int main() {
    std::signal(SIGINT,  handleSignal);
    std::signal(SIGTERM, handleSignal);

    DroneApp app;
    gApp = &app;

    if (!app.init()) {
        std::cerr << "Drone init failed — motors will not run\n";
        return 1;
    }

    app.run();   // blocks until stop() is called or Ctrl+C
    return 0;
}
