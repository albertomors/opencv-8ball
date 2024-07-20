/*
AUTHOR: Morselli Alberto 
FILE: frameHandler.cpp
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
    //std::cout << "table_color: H=" << table.hue_color << " BGR=" << table.bgr_color << std::endl;
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
    cv::imshow("seg mask", table.seg_mask);
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

void frameHandler::project(const cv::Mat& frame){
    projecter.projectBalls(frame, tracker.centers, tracker.trajectories, this->starting_ids, this->table_corners);
}