/*
    AUTHOR: Girardello Sofia
    DATE: 2024-07-21 
    FILE: ballDetector.h
    DESCRIPTION: Description of the class that detects and classificates the balls.

    CLASSES:
    - class ballDetector: Class for detecting and classifying balls.

    MAIN FUNCTIONS:
    - ballDetector(): Constructor to initialize the ballDetector object.
    - void detectBalls(...): Handles the detection and calls the other functions.
    - void applyColourDetection(...): Performs detection using Hough Transform on colour masks.
    - void selectBalls(...): Select just the acceptable balls using colour thresholding masks.
    - BallPattern analyzeBallPattern(...): Analyzes the ball pattern based on its appearance.
    - void classifyBalls(...): Classifies each ball given its colour and pattern analytics.
    - void detectBallsFinalFrame(...): Detects balls in the final frame and matches with tracker centers.
    - void saveInfo(...): Stores important information about each single selected ball.

    ADDITIONAL FUNCTIONS: 
    - enhanceContrast(...): Enhances the contrast of the input image using CLAHE (Contrast Limited Adaptive Histogram Equalization) in the LAB color space. Improves the visibility of features in the image.
    - detectedBallsData(...): Constructs a matrix with information about detected balls, including their bounding boxes and IDs.
    - createLabeledImage(...): Creates a labeled image that visualizes detected balls with their corresponding IDs.

    EXAMPLES:
    - Input: A frame from a video feed with balls visible.
    - Output: A frame with detected circles overlaid, showing both the detected circles and their classes.
*/

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>

#ifndef BALLDDETECTION_INCLUDED
  #define BALLDETECTION_INCLUDED

  struct BallPattern {
    double whitePercentage;
    double blackPercentage;
    int id;
  };

  class ballDetector{

    /*
    Class ballDetector :
    */

    private: 

    std::vector<cv::Rect> bboxes;
    
    public:

    cv::Mat table_roi;
    std::vector<cv::Point2f> centers;
    std::vector<cv::Rect> balls;
    std::vector<int> id_balls;
    cv::Mat bbox_data;
    cv::Mat classification_res;


    explicit ballDetector();

    void detectBalls(const cv::Mat& currentFrame, const cv::Mat& ROI, const std::vector<cv::Point2f> table_corners);
    void applyColourDetection(cv::Mat& frame, cv::Mat& colour_mask, std::vector<cv::Vec3f>& circles);
    std::vector<BallPattern> selectBalls(const cv::Mat& ROI, const cv::Mat& mask, const std::vector<cv::Vec3f>& circle, const std::vector<cv::Point2f> table_corners);
    BallPattern analyzeBallPattern(const cv::Mat& ballROI, const cv::Mat& circleMask);
    void classifyBalls(std::vector<BallPattern>& ballPatterns);
    void detectBallsFinalFrame(const cv::Mat& frame, const cv::Mat& ROI, const std::vector<cv::Point2f>& trackerCenters, const std::vector<int>& trackerIDs, const std::vector<cv::Point2f>& table_corners);
    void saveInfo(const cv::Point center, const int radius);

  };
#endif