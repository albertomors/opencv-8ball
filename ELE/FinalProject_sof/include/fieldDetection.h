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
    //to have the area
    std::vector<cv::Point> fieldArea;
    
    public:

    explicit fieldDetector(const cv::Mat& currentFrame); //i added &
    void detectField();

    // method to retrieve detected points and area
    std::vector<cv::Point> getEdgePoints() const;
    std::vector<cv::Point> getFieldArea() const;

  };
#endif