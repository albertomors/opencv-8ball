
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <vector>

#include "metrics.h"
#include "videoHandler.h"

// Function to create a dummy mask (for test)
cv::Mat createDummyMask(int rows, int cols) {
    // Create an empty mask with the specified dimensions
    cv::Mat dummyMask(rows, cols, CV_8UC1);

    // Initialize the mask with random values for testing
    cv::randu(dummyMask, cv::Scalar(0), cv::Scalar(6)); // Random values between 0 and 5

    return dummyMask;
}


int main() {
    std::string folder_name = "game1_clip1"; // Replace with your actual folder name

    std::string mask_path = "Dataset/" + folder_name + "/masks/";

    //std::cout << "Mask path: " << mask_path << std::endl;

    // Use public member functions to access masks and bounding boxes
    cv::Mat groundTruthFirstFrame = cv::imread(mask_path + "frame_first.png", cv::IMREAD_GRAYSCALE);
    cv::Mat groundTruthLastFrame = cv::imread(mask_path + "frame_last.png", cv::IMREAD_GRAYSCALE);

    // Check if masks are not empty
    if (groundTruthFirstFrame.empty() || groundTruthLastFrame.empty()) {
        std::cerr << "One or both masks are empty." << std::endl;
        return 1; // Exit the program indicating an error
    }

    int width = 1024;
    int height = 576;

    // Create an empty mask image of specified size
    //cv::Mat groundTruthFirstFrame = cv::Mat::zeros(height, width, CV_8UC1);
    //cv::Mat predictionFirstFrame = cv::Mat::zeros(height, width, CV_8UC1);
    //cv::Mat groundTruthLastFrame = cv::Mat::zeros(height, width, CV_8UC1);
    //cv::Mat predictionLastFrame = cv::Mat::zeros(height, width, CV_8UC1);

    cv::Mat predictionFirstFrame = createDummyMask(height, width);
    cv::Mat predictionLastFrame = createDummyMask(height, width);


    std::vector<std::pair<cv::Mat, cv::Mat>> videoSegMasks;
    videoSegMasks.push_back(std::make_pair(groundTruthFirstFrame, predictionFirstFrame));
    videoSegMasks.push_back(std::make_pair(groundTruthLastFrame, predictionLastFrame));

    std::cout << "# pairs:" << videoSegMasks.size() << std::endl;
    // Compute mIoU
    int numClasses = 6; // Number of classes
    double mIoU = compute_mIoU(videoSegMasks, numClasses);
    //calculateMeanIoUForVideo
    

    //std::cout << "Mean IoU: " << mIoU << std::endl;
    std::cout << "Mean IoU: " << mIoU << std::endl;



    return 0;
}