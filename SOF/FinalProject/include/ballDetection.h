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
    std::vector<cv::Point> field;
    bool draw;
    std::vector<cv::Rect> bboxes;

    
    public:

    std::vector<cv::Point2f> centers;
    std::vector<cv::Rect> balls;
    std::vector<int> id_balls;
    cv::Mat table_roi;
    cv::Mat bbox_data;
    cv::Mat classification_res;


    //explicit ballDetector(const cv::Mat currentFrame, std::vector<cv::Point> edgePoints);
    explicit ballDetector();

    void detectBalls(const cv::Mat& currentFrame, const cv::Mat& ROI, const float color);

    void drawDetection();

    int analyzeBallPattern(const cv::Mat& ballROI, const cv::Point& center, int radius);

  };
#endif