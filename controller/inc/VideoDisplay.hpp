#pragma once
#include <opencv2/opencv.hpp>
#include <string>

// Reads frames from Avedio USB capture dongle (/dev/video0)
//  receives 5.8 GHz FPV video from the drone
class VideoDisplay {
public:
    explicit VideoDisplay(const std::string& device = "/dev/video0");
    ~VideoDisplay();

    bool init();
    void update();   // grab + display one frame

private:
    std::string      device_;
    cv::VideoCapture cap_;
    cv::Mat          frame_;

    static constexpr const char* WIN_NAME = "Pringle FPV";
};
