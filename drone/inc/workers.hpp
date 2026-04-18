#ifndef WORKERS_HPP
#define WORKERS_HPP

#include "main.hpp"
#include "MPU6050.h"

#include <opencv2/opencv.hpp>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include "RadioManager.hpp"

// declarations for worker threads
void motor_worker();
void camera_worker();
void listen_worker();

#endif