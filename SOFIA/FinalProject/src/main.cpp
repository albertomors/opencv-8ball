#include "fieldDetection.h"
#include "ballDetection.h"
#include <iostream>

int main() {
    // Load a sample frame (for example, a frame from a video or an image of a billiard table)
    cv::Mat frame = cv::imread("Dataset/game1_clip1/frames/frame_first.png");

    // Check if the image is loaded successfully
    if (frame.empty()) {
        std::cerr << "Error loading the image" << std::endl;
        return -1;
    }

    ballDetector detector(frame);
    detector.detectBalls();

    cv::waitKey(0);
}