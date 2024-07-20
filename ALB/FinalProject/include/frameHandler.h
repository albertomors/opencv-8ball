/*
AUTHOR: Morselli Alberto 
FILE: frameHandler.h
*/

#ifndef FRAMEHANDLER_INCLUDED
#define FRAMEHANDLER_INCLUDED

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>

#include "table.h"
#include "ballDetection.h"
#include "trajectoryTracking.h"
#include "trajectoryProjection.h"

class frameHandler{

private:

    tableDetector table;
    ballDetector detector;
    trajectoryTracker tracker;
    trajectoryProjecter projecter;

public:

    explicit frameHandler();

    void detect_table(const cv::Mat& frame);
    void detect_balls(const cv::Mat& frame);
    void initializeTrackers(const cv::Mat& frame);
    void updateTrackers(const cv::Mat& frame);
    void project(const cv::Mat& frame);

    cv::Mat draw_frame(const cv::Mat& frame);
};

#endif