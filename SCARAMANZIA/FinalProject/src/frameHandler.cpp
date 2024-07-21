/*
    AUTHOR: Fresco Eleonora
    DATE: 2024-07-21
    FILE: frameHandler.cpp
    DESCRIPTION: Implements the frame handler class to manage table detection, ball detection, ball tracking, and trajectory projection in a video feed.

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

    EXAMPLES:
    - Input: A frame from a video feed with a table and balls visible.
    - Output: Various operations such as detecting the table, detecting balls, initializing and updating trackers, drawing table borders, and projecting ball trajectories.
*/

#include "frameHandler.h"
#include "table.h"
#include "trajectoryTracking.h"
#include "trajectoryProjection.h"

frameHandler::frameHandler(){
    this->table = tableDetector();
    this->detector = ballDetector();
    this->tracker = trajectoryTracker();
    this->projecter = trajectoryProjecter();
}

void frameHandler::detect_table(const cv::Mat& frame){
    table.find_table(frame);
    //--Debug  std::cout << "table_color: H=" << table.hue_color << " BGR=" << table.bgr_color << std::endl;
}

void frameHandler::save_table_corners(){
    this->table_corners = table.corners;
}

void frameHandler::detect_balls(const cv::Mat& frame){
    detector.detectBalls(frame, table.seg_mask, this->table_corners);
    this->bbox_data = detector.bbox_data;
    this->classification_res = detector.classification_res;
}

void frameHandler::detect_balls_final(const cv::Mat& frame){
    detector.detectBallsFinalFrame(frame, table.seg_mask, tracker.centers, this->starting_ids, this->table_corners);
    this->bbox_data = detector.bbox_data;
    this->classification_res = detector.classification_res;
}

void frameHandler::initializeTrackers(const cv::Mat& frame){
    tracker.initializeTrackers(frame, detector.balls);      
}

void frameHandler::save_ids(){
    this->starting_ids = detector.id_balls;
}

void frameHandler::updateTrackers(const cv::Mat& frame){
    tracker.updateTrackers(frame);
}

cv::Mat frameHandler::draw_frame(const cv::Mat& frame){
    return table.draw_borders(frame);
}

cv::Mat frameHandler::project(const cv::Mat& frame){
    return projecter.projectBalls(frame, tracker.centers, tracker.trajectories, this->starting_ids, this->table_corners);
}