#ifndef WORKERS_HPP
#define WORKERS_HPP

#include "main.hpp"
#include <opencv2/opencv.hpp>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/fb.h>

// declarations for worker threads
void motor_worker();
void camera_worker();
void listen_worker();

#endif