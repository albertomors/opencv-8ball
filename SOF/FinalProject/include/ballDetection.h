#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>


#ifndef BALLDDETECTION_INCLUDED
  #define BALLDETECTION_INCLUDED

  class ballDetector{

    /*
    Class ballDetector :
    */

    private: 

    cv::Mat frame;
    cv::Mat table_roi;
    std::vector<cv::Point> field;
    bool draw;
    
    public:

    std::vector<cv::Point2f> centers;
    std::vector<cv::Rect> balls;


    //explicit ballDetector(const cv::Mat currentFrame, std::vector<cv::Point> edgePoints);
    explicit ballDetector();

    void detectBalls(const cv::Mat& currentFrame, const cv::Mat& ROI, const float field_color, const cv::Mat& seg_mask);

    void drawDetection();

    bool analyzeBallPattern(const cv::Mat& ballROI, const cv::Point& center, int radius);

  };
#endif