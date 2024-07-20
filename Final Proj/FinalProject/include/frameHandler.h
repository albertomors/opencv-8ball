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

    std::vector<cv::Point2f> table_corners;
    std::vector<int> starting_ids;

public:

    cv::Mat bbox_data;
    cv::Mat classification_res;

    explicit frameHandler();

    void detect_table(const cv::Mat& frame);
    void save_table_corners();
    void detect_balls(const cv::Mat& frame);
    void detect_balls_final(const cv::Mat& frame);
    void initializeTrackers(const cv::Mat& frame);
    void save_ids();
    void updateTrackers(const cv::Mat& frame);
    void project(const cv::Mat& frame);

    cv::Mat draw_frame(const cv::Mat& frame);
};

#endif