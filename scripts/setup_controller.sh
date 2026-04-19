#!/usr/bin/env bash
set -euo pipefail

echo "=== Controller RPi Setup ==="

sudo apt update
sudo apt install -y \
    cmake \
    build-essential \
    liblgpio-dev \
    libopencv-dev \
    libv4l-dev \
    joystick

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
echo "To build:"
echo "  cd controller && mkdir -p build && cd build && cmake .. && make -j\$(nproc)"
echo "To run:"
echo "  sudo ./pringle_controller"
