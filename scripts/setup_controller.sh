#!/usr/bin/env bash
# setup_controller.sh — run once on the controller RPi
set -euo pipefail

echo "=== Controller RPi Setup ==="

sudo apt update
sudo apt install -y \
    cmake \
    build-essential \
    liblgpio-dev \
    libopencv-dev \
    libv4l-dev \
    joystick          # provides jstest for verifying gamepad axis mapping

# ── Build and install RF24 with LGPIO driver ────────────────────────────────
WORKDIR="$HOME/rf24_build"
mkdir -p "$WORKDIR"
cd "$WORKDIR"

if [ ! -d "RF24" ]; then
    git clone https://github.com/nRF24/RF24.git
fi

cd RF24
mkdir -p build && cd build
cmake .. -D RF24_DRIVER=SPIDEV
make -j"$(nproc)"
sudo make install
sudo ldconfig

echo ""
echo "=== Setup complete ==="
echo ""
echo "To verify your gamepad axis mapping before running:"
echo "  jstest /dev/input/js0"
echo "  (check which axis numbers move when you push each stick)"
echo "  Update AXIS_* constants in controller/include/GamepadReader.hpp if needed."
echo ""
echo "To build:"
echo "  cd controller && mkdir -p build && cd build && cmake .. && make -j\$(nproc)"
echo ""
echo "To run (must be root for GPIO access):"
echo "  sudo ./r6_controller"
