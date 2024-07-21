/*
    AUTHOR: Girardello Sofia
    DATE: 2024-07-21
    FILE: trajectoryTracking.h
    DESCRIPTION: Defines the trajectory tracking class using OpenCV trackers.

    CLASSES:
    - class trajectoryTracker: Class for tracking the trajectories of multiple objects.

    MAIN FUNCTIONS:
    - trajectoryTracker(): Constructor to initialize the trajectoryTracker object.
    - void initializeTrackers(...): Initializes trackers for the given bounding boxes.
    - void updateTrackers(...): Updates the trackers with the current frame and stores the centers and trajectories.
*/

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

    std::vector<std::vector<cv::Point2f>> ballTrajectories;
    std::vector<cv::Ptr<cv::Tracker>> trackers;
    
    public:

    std::vector<cv::Point2f> centers;
    std::vector<std::vector<cv::Point2f>> trajectories;

    explicit trajectoryTracker();

    void initializeTrackers(const cv::Mat& frame, const std::vector<cv::Rect>& centers);
    void updateTrackers(const cv::Mat& frame);


  };
#endif