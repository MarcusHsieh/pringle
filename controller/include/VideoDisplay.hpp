#pragma once
#include <opencv2/opencv.hpp>
#include <string>

// Reads frames from the Avedio USB capture dongle (/dev/video0),
// which receives the 5.8 GHz FPV video from the drone,
// and displays them full-screen on the 5" HDMI display.
class VideoDisplay {
public:
    explicit VideoDisplay(const std::string& device = "/dev/video0");
    ~VideoDisplay();

    bool init();
    void update();   // grab + display one frame; non-blocking

private:
    std::string      device_;
    cv::VideoCapture cap_;
    cv::Mat          frame_;

    static constexpr const char* WIN_NAME = "R6 Drone";
};
