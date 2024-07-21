#include "ballDetection.h"
#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    // Load the current frame and ROI images
    std::string folder_name = "game1_clip1";
    std::string frame_path = "Dataset/" + folder_name + "/frames/";

    cv::Mat currentFrame = cv::imread(frame_path + "frame_first.png");
    cv::Mat ROI = cv::imread("path_to_ROI.jpg", cv::IMREAD_GRAYSCALE);

    // Check if the images are loaded successfully
    if (currentFrame.empty() || ROI.empty()) {
        std::cerr << "Error: Could not load the images!" << std::endl;
        return -1;
    }

    // Initialize the ball detector
    ballDetector detector;

    // Detect balls in the current frame using the ROI and a sample color value
    float sampleColor = 30.0;  // Example color value
    detector.detectBalls(currentFrame, ROI, sampleColor);

    return 0;
}