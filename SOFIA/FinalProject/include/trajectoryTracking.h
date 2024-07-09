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
    
    public:

    explicit trajectoryTracker(const cv::Mat currentFrame);

    void track();

    void drawTrajectory();

  };
#endif