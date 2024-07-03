#include "fieldDetection.h"

int main() {
    // Load a sample frame (for example, a frame from a video or an image of a billiard table)
    //cv::Mat frame = cv::imread("../../FinalProject/Dataset/game1_clip1/frames/frame_first.png");
    cv::Mat frame = cv::imread("Dataset/game1_clip1/frames/frame_first.png");

    // Check if the image is loaded successfully
    if (frame.empty()) {
        std::cerr << "Error loading the image" << std::endl;
        return -1;
    }

    // Create a fieldDetector object
    fieldDetector detector(frame);

    // Detect the field boundaries
    detector.detectField();

    return 0;
}