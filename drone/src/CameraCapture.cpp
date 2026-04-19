#include "CameraCapture.hpp"
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <chrono>

using namespace std::chrono_literals;

CameraCapture::CameraCapture(const std::string& device) : device_(device) {}

CameraCapture::~CameraCapture() {
    stop();
    if (fbFd_ >= 0) ::close(fbFd_);
}

bool CameraCapture::init() {
    // ── Framebuffer ───────────────────────────────────────────────────────────
    fbFd_ = ::open("/dev/fb0", O_RDWR);
    if (fbFd_ < 0) {
        std::cerr << "[Camera] Cannot open /dev/fb0\n"
                  << "         Is 'enable_tvout=1' in /boot/config.txt? (reboot required)\n";
        return false;
    }

    // Read actual framebuffer resolution from kernel
    struct fb_var_screeninfo vinfo{};
    ::ioctl(fbFd_, FBIOGET_VSCREENINFO, &vinfo);
    fbWidth_  = static_cast<int>(vinfo.xres);
    fbHeight_ = static_cast<int>(vinfo.yres);
    std::cout << "[Camera] Framebuffer: " << fbWidth_ << "x" << fbHeight_
              << " @ " << vinfo.bits_per_pixel << "bpp\n";

    // ── USB Camera ────────────────────────────────────────────────────────────
    cap_.open(device_, cv::CAP_V4L2);
    if (!cap_.isOpened()) {
        std::cerr << "[Camera] Cannot open " << device_ << "\n";
        return false;
    }
    cap_.set(cv::CAP_PROP_FRAME_WIDTH,  fbWidth_);
    cap_.set(cv::CAP_PROP_FRAME_HEIGHT, fbHeight_);
    cap_.set(cv::CAP_PROP_BUFFERSIZE,   1);   // always grab latest frame

    running_ = true;
    thread_  = std::thread(&CameraCapture::captureLoop, this);
    std::cout << "[Camera] Streaming to 3.5mm composite jack...\n";
    return true;
}

void CameraCapture::stop() {
    running_ = false;
    if (thread_.joinable()) thread_.join();
    if (cap_.isOpened()) cap_.release();
}

void CameraCapture::captureLoop() {
    cv::Mat frame, bgra;

    while (running_) {
        cap_ >> frame;
        if (frame.empty()) continue;

        // Convert BGR → BGRA (4 bytes/pixel — matches RPi's 32bpp framebuffer)
        cv::cvtColor(frame, bgra, cv::COLOR_BGR2BGRA);

        // Write directly to framebuffer hardware memory
        ::lseek(fbFd_, 0, SEEK_SET);
        ::write(fbFd_, bgra.data, bgra.total() * bgra.elemSize());

        // ~30 fps cap — composite output refreshes at 30 Hz (NTSC)
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
}
