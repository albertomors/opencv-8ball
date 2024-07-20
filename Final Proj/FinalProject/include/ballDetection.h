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

    cv::Mat frame;
    cv::Mat roi;
    std::vector<cv::Point> field;
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

    void detectBallsFinalFrame(const cv::Mat& frame, const cv::Mat& ROI, const std::vector<cv::Point2f>& trackerCenters, const std::vector<int>& trackerIDs, const std::vector<cv::Point2f>& table_corners);

    BallPattern analyzeBallPattern(const cv::Mat& ballROI, const cv::Point& center, int radius);

  };
#endif