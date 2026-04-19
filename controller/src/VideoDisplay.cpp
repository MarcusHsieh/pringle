#include "VideoDisplay.hpp"
#include <iostream>

VideoDisplay::VideoDisplay(const std::string& device) : device_(device) {}

VideoDisplay::~VideoDisplay() {
    if (cap_.isOpened()) cap_.release();
    cv::destroyAllWindows();
}

bool VideoDisplay::init() {
    // CAP_V4L2 is explicit so OpenCV doesn't waste time probing other backends
    cap_.open(device_, cv::CAP_V4L2);
    if (!cap_.isOpened()) {
        std::cerr << "[Video] Cannot open " << device_ << "\n";
        std::cerr << "[Video] Is the Avedio USB capture dongle plugged in?\n";
        return false;
    }

    // Ask the capture card for 720×480 at 30fps.
    // The Avedio may override these with its native resolution — that's fine.
    cap_.set(cv::CAP_PROP_FRAME_WIDTH,  720);
    cap_.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    cap_.set(cv::CAP_PROP_FPS,          30);
    cap_.set(cv::CAP_PROP_BUFFERSIZE,   1);  // 1-frame buffer = minimum latency

    cv::namedWindow(WIN_NAME, cv::WINDOW_NORMAL);
    cv::setWindowProperty(WIN_NAME, cv::WND_PROP_FULLSCREEN, cv::WINDOW_FULLSCREEN);

    std::cout << "[Video] Capture opened on " << device_ << "\n";
    return true;
}

void VideoDisplay::update() {
    // grab() queues a frame without decoding it (fast).
    // retrieve() then decodes the already-grabbed frame.
    // Calling grab() repeatedly without retrieve() effectively drops stale frames,
    // keeping latency minimal even if the main loop is briefly slow.
    if (!cap_.grab()) return;
    cap_.retrieve(frame_);
    if (frame_.empty()) return;

    cv::imshow(WIN_NAME, frame_);
    cv::waitKey(1);   // 1ms — required to pump OpenCV's GUI event loop
}
