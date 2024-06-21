#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>


#ifndef FIELDDETECTION_INCLUDED
  #define FIELDDETECTION_INCLUDED

  class fieldDetector{

    /*
    Class fieldDetector :
    */

    private: 

    cv::Mat frame;
    std::vector<cv::Point> edgePoints;
    
    public:

    explicit fieldDetector(const cv::Mat currentFrame);

    void detectField();

  };
#endif