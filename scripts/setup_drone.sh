#!/usr/bin/env bash
# setup_drone.sh — run once on the drone RPi
set -euo pipefail

echo "=== Drone RPi Setup ==="

sudo apt update
sudo apt install -y \
    cmake \
    build-essential \
    liblgpio-dev \
    libopencv-dev \
    libv4l-dev

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

# ── Composite video config ───────────────────────────────────────────────────
CONFIG=/boot/config.txt
if ! grep -q "enable_tvout=1" "$CONFIG"; then
    echo "" | sudo tee -a "$CONFIG"
    echo "# Composite video output for FPV transmitter" | sudo tee -a "$CONFIG"
    echo "enable_tvout=1" | sudo tee -a "$CONFIG"
    echo ""
    echo "NOTE: enable_tvout=1 added to $CONFIG"
    echo "      REBOOT REQUIRED before camera output will work."
else
    echo "enable_tvout=1 already set in $CONFIG — no changes needed."
fi

echo ""
echo "=== Setup complete ==="
echo ""
echo "To build:"
echo "  cd drone && mkdir -p build && cd build && cmake .. && make -j\$(nproc)"
echo ""
echo "To run (must be root for GPIO access):"
echo "  sudo ./r6_drone"
echo ""
echo "To verify composite video manually (quick Python test):"
echo "  python3 -c \""
echo "  import cv2, mmap, struct, fcntl, os"
echo "  FBIOGET_VSCREENINFO = 0x4600"
echo "  fd = open('/dev/fb0','rb+'); buf = bytearray(160); fcntl.ioctl(fd, FBIOGET_VSCREENINFO, buf)"
echo "  w,h = struct.unpack_from('II', buf, 0); print(f'fb0: {w}x{h}')"
echo "  \""
