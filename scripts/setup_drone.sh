#!/usr/bin/env bash
set -euo pipefail

echo "=== Drone RPi Setup ==="

sudo apt update
sudo apt install -y \
    cmake \
    build-essential \
    liblgpio-dev \
    libopencv-dev \
    libv4l-dev

WORKDIR="$HOME/rf24_build"
mkdir -p "$WORKDIR"
cd "$WORKDIR"

if [ ! -d "RF24" ]; then
    git clone https://github.com/nRF24/RF24.git
fi

cd RF24
mkdir -p build && cd build
cmake .. -D RF24_DRIVER=LGPIO
make -j"$(nproc)"
sudo make install
sudo ldconfig

CONFIG=/boot/config.txt
if ! grep -q "enable_tvout=1" "$CONFIG"; then
    echo -e "\n# Composite video output for FPV transmitter\nenable_tvout=1" | sudo tee -a "$CONFIG" > /dev/null
    echo "NOTE: Added enable_tvout=1 to $CONFIG. REBOOT REQUIRED."
fi

echo ""
echo "=== Setup complete ==="
echo ""
echo "To build:"
echo "  cd drone && mkdir -p build && cd build && cmake .. && make -j\$(nproc)"