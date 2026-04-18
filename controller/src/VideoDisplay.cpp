#include "VideoDisplay.hpp"
#include <iostream>

VideoDisplay::VideoDisplay(const std::string& device) : device_(device) {}

VideoDisplay::~VideoDisplay() {
    if (cap_.isOpened()) cap_.release();
    cv::destroyAllWindows();
}

bool VideoDisplay::init() {
    cap_.open(device_, cv::CAP_V4L2); // Video4Linux2 
    if (!cap_.isOpened()) {
        std::cerr << "[Video] Cannot open " << device_ << "\n";
        return false;
    }

    // 720×480 @ 30fps
    cap_.set(cv::CAP_PROP_FRAME_WIDTH,  720);
    cap_.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    cap_.set(cv::CAP_PROP_FPS,          30);
    cap_.set(cv::CAP_PROP_BUFFERSIZE,   1);  // 1 frame buffer = min latency

    cv::namedWindow(WIN_NAME, cv::WINDOW_NORMAL);
    cv::setWindowProperty(WIN_NAME, cv::WND_PROP_FULLSCREEN, cv::WINDOW_FULLSCREEN);

    std::cout << "[Video] Capture opened on " << device_ << "\n";
    return true;
}

void VideoDisplay::update() {
    if (!cap_.grab()) return;
    cap_.retrieve(frame_);
    if (frame_.empty()) return;

    cv::imshow(WIN_NAME, frame_);
    cv::waitKey(1);   // 1ms
}
