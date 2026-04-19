#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <opencv2/opencv.hpp>

// Reads frames from the USB camera and writes them to /dev/fb0
// (RPi framebuffer → 3.5mm composite jack → TS832 FPV transmitter).
//
// Runs in its own thread. Uses write() directly to fb_fd (same approach
// as teammate's camera_worker, no mmap needed).
//
// Assumes 32bpp BGRA framebuffer (RPi default with enable_tvout=1).
// REQUIRED in /boot/config.txt:
//   enable_tvout=1
class CameraCapture {
public:
    explicit CameraCapture(const std::string& device = "/dev/video0");
    ~CameraCapture();

    bool init();
    bool isRunning() const { return running_.load(); }
    void stop();

private:
    void captureLoop();

    std::string        device_;
    cv::VideoCapture   cap_;
    int                fbFd_    = -1;
    int                fbWidth_ = 640;
    int                fbHeight_= 480;

    std::thread       thread_;
    std::atomic<bool> running_{false};
};
