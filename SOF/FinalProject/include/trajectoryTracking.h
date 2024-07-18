#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>


#ifndef TRAJECTORYTRACKING_INCLUDED
  #define TRAJECTORYTRACKING_INCLUDED

  class trajectoryTracker{

    /*
    Class trajectoryTracker :
    */

    private: 

    cv::Mat frame;
    //cv::Mat previousFrame;
    cv::Mat segmentation;
    std::vector<std::vector<cv::Point2f>> ballTrajectories;
    std::vector<cv::Ptr<cv::Tracker>> trackers;
    
    public:

    explicit trajectoryTracker();

    void initializeTrackers(const cv::Mat& frame, const std::vector<cv::Rect>& centers);
    void updateTrackers(const cv::Mat& frame);
    //void drawTrajectories(cv::Mat& frame);

  };
#endif