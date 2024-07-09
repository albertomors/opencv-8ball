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
    std::vector<cv::Rect> balls;
    bool draw;
    
    public:

    //explicit ballDetector(const cv::Mat currentFrame, std::vector<cv::Point> edgePoints);
    explicit ballDetector(const cv::Mat& currentFrame);

    void detectBalls();

    void drawDetection();

    bool analyzeBallPattern(const cv::Mat& ballROI, const cv::Point& center, int radius);

  };
#endif