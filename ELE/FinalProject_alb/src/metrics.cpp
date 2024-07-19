/*
AUTHOR: Morselli Alberto 
FILE: metrics.cpp

-called by videoHandler
-compute metrics, show and save them
*/

#include "metrics.h"

void compute_mAP(std::vector<cv::Rect>& balls){
    //TODO
    return;
}

/*void compute_mIoU(cv::Mat& seg_mask){
    //TODO
    return;
}*/

// Function to calculate IoU for a single class (from 0 to 5)
double calculateIoU(const cv::Mat& groundTruth, const cv::Mat& prediction, int classId) {
    cv::Mat intersection, union_;

    // Create binary masks for the class
    cv::Mat gtBinary = (groundTruth == classId);
    cv::Mat predBinary = (prediction == classId);

    // Calculate intersection and union
    cv::bitwise_and(gtBinary, predBinary, intersection);
    cv::bitwise_or(gtBinary, predBinary, union_);

    // Calculate TP, FP, and FN
    double TP = cv::countNonZero(intersection);
    double FP = cv::countNonZero(predBinary) - TP;
    double FN = cv::countNonZero(gtBinary) - TP;

    double unionArea = TP + FP + FN;
    std::cout << "TP: " << TP << std::endl;
    std::cout << "FP: " << FP << std::endl;
    std::cout << "FN: " << FN << std::endl;

    //if (unionArea == 0) return 1.0; // To handle cases where both are empty
    if (unionArea == 0) {
        unionArea = 1.0; // Set unionArea to 1 to handle cases where both are empty
    }

    return TP / unionArea;
}

// Function to calculate mean IoU for a video, so for both first and last frames
/*double calculateMeanIoUForVideo(const cv::Mat& groundTruthFirstFrame, const cv::Mat& predictionFirstFrame, const cv::Mat& groundTruthLastFrame, const cv::Mat& predictionLastFrame, int numClasses) {
    double totalIoU = 0.0;

    for (int classId = 0; classId < numClasses; ++classId) {
        double iouFirstFrame = calculateIoU(groundTruthFirstFrame, predictionFirstFrame, classId);
        double iouLastFrame = calculateIoU(groundTruthLastFrame, predictionLastFrame, classId);
        double meanIoU = (iouFirstFrame + iouLastFrame) / 2.0;
        totalIoU += meanIoU;
    }

    return totalIoU / numClasses;
}

// Function to compute the average mIoU across all videos -- NO
double compute_mIoU(const std::vector<std::pair<cv::Mat, cv::Mat>>& videoSegMasks, int numClasses) {
    double overallMeanIoU = 0.0;
    int videoCount = videoSegMasks.size();

    for (const auto& segMaskPair : videoSegMasks) {
        const cv::Mat& groundTruthFirstFrame = segMaskPair.first;
        const cv::Mat& predictionFirstFrame = segMaskPair.second;

        const cv::Mat& groundTruthLastFrame = segMaskPair.first;
        const cv::Mat& predictionLastFrame = segMaskPair.second;

        double meanIoUForVideo = calculateMeanIoUForVideo(groundTruthFirstFrame, predictionFirstFrame, groundTruthLastFrame, predictionLastFrame, numClasses);
        overallMeanIoU += meanIoUForVideo;
    }

    return overallMeanIoU / videoCount;
}*/

// Function to compute the average mIoU for the considered video
double compute_mIoU(const std::vector<std::pair<cv::Mat, cv::Mat>>& videoSegMasks, int numClasses) {
    // Ensure we have at least one pair of masks
    /*if (videoSegMasks.empty()) {
        std::cerr << "No mask pairs available for mIoU computation." << std::endl;
        return 0.0;
    }*/

    // Ensure we have exactly two pairs of masks
    if (videoSegMasks.size() != 2) {
        std::cerr << "Exactly two mask pairs are required for mIoU computation." << std::endl;
        return 0.0;
    }

    // Take the first pair of masks (assuming the vector contains pairs for a single video)
    /*const auto& segMaskPair = videoSegMasks[0];

    const cv::Mat& groundTruthFirstFrame = segMaskPair.first;
    const cv::Mat& predictionFirstFrame = segMaskPair.second;
    const cv::Mat& groundTruthLastFrame = segMaskPair.first;
    const cv::Mat& predictionLastFrame = segMaskPair.second;*/
    const auto& firstFramePair = videoSegMasks[0];
    const auto& lastFramePair = videoSegMasks[1];

    const cv::Mat& groundTruthFirstFrame = firstFramePair.first;
    const cv::Mat& predictionFirstFrame = firstFramePair.second;
    const cv::Mat& groundTruthLastFrame = lastFramePair.first;
    const cv::Mat& predictionLastFrame = lastFramePair.second;

    // Check that all masks are of the same size
    if (groundTruthFirstFrame.size() != predictionFirstFrame.size() ||
        groundTruthFirstFrame.size() != groundTruthLastFrame.size() ||
        groundTruthFirstFrame.size() != predictionLastFrame.size()) {
        std::cerr << "All mask pairs must be of the same size." << std::endl;
        return 0.0;
    }

    // Check that each predicted mask has the same type as the corresponding ground truth mask
    if (groundTruthFirstFrame.type() != predictionFirstFrame.type() ||
        groundTruthLastFrame.type() != predictionLastFrame.type()) {
        std::cerr << "Prediction mask types must match the ground truth mask types." << std::endl;
        return 0.0;
    }
    
    double totalMeanIoU = 0.0;

    for (int classId = 0; classId < numClasses; ++classId) {
        std::cout << "Class #: " << classId << std::endl;
        double iouFirstFrame = calculateIoU(groundTruthFirstFrame, predictionFirstFrame, classId);
        double iouLastFrame = calculateIoU(groundTruthLastFrame, predictionLastFrame, classId);
        double meanIoU = (iouFirstFrame + iouLastFrame) / 2.0;
        std::cout << "Class #: " << classId << " first and last/2: " << meanIoU << std::endl;
        totalMeanIoU += meanIoU;
        std::cout << "Till Class #: " << classId << " total numerator: " << totalMeanIoU << std::endl;
    }

    /*double totalMeanIoU = 0.0;
    //int numPairs = videoSegMasks.size();

    // Iterate over all pairs of masks
    for (const auto& segMaskPair : videoSegMasks) {
        const cv::Mat& groundTruthFirstFrame = segMaskPair.first;
        const cv::Mat& predictionFirstFrame = segMaskPair.second;
        const cv::Mat& groundTruthLastFrame = segMaskPair.first;
        const cv::Mat& predictionLastFrame = segMaskPair.second;

        for (int classId = 0; classId < numClasses; ++classId) {
            double iouFirstFrame = calculateIoU(groundTruthFirstFrame, predictionFirstFrame, classId);
            double iouLastFrame = calculateIoU(groundTruthLastFrame, predictionLastFrame, classId);
            double meanIoU = (iouFirstFrame + iouLastFrame) / 2.0;
            totalMeanIoU += meanIoU;
        }
    }*/

    return totalMeanIoU / numClasses;
}


// Compute mIoU 
/*double compute_mIoU(const std::vector<std::pair<cv::Mat, cv::Mat>>& videoSegMasks, int numClasses) {
    double totalIoU = 0.0;

    for (int classId = 0; classId < numClasses; ++classId) {
        double iouFirstFrame = calculateIoU(groundTruthFirstFrame, predictionFirstFrame, classId);
        double iouLastFrame = calculateIoU(groundTruthLastFrame, predictionLastFrame, classId);
        double meanIoU = (iouFirstFrame + iouLastFrame) / 2.0;
        totalIoU += meanIoU;
    }

    return totalIoU / numClasses;
}*/
