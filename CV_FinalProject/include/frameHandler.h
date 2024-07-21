/*
    AUTHOR: Morselli Alberto
    DATE: 2024-07-21
    FILE: frameHandler.h
    DESCRIPTION: Created the class that handles all the operations to perform on the single frame.

    CLASSES:
    - class frameHandler: Class for handling frames, including detecting tables and balls, initializing and updating trackers, and projecting ball trajectories.

    MAIN FUNCTIONS:
    - frameHandler(): Constructor to initialize the frameHandler object.
    - void detect_table(...): Detects the table in the given frame.
    - void detect_balls(...): Detects balls in the given frame.
    - void detect_balls_final(...): Detects balls in the final frame and matches them with tracker centers.
    - void initializeTrackers(...): Initializes trackers for the detected balls.
    - void updateTrackers(...): Updates the trackers with the current frame.
    - cv::Mat draw_frame(...): Draws the borders of the table on the given frame.
    - void project(...): Projects the ball trajectories on the given frame.

    ADDITIONAL FUNCTIONS:
    - save_table_corners(): Stores the corners of the detected table for later use.
    - save_ids(): Stores the IDs of the detected balls for later use.
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
    cv::Mat project(const cv::Mat& frame);
    cv::Mat draw_frame(const cv::Mat& frame);

};

#endif