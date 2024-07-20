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
    //this->projecter = trajectoryProjecter(std::vector<cv::Point2f>());
    this->projecter = trajectoryProjecter();
}

void frameHandler::detect_table(const cv::Mat& frame){
    table.find_table(frame);
    std::cout << "table_color: H=" << table.hue_color << " BGR=" << table.bgr_color << std::endl;
    //ONLY after having detected corners we can project
    //this->projecter = trajectoryProjecter(table.corners);
}

void frameHandler::detect_balls(const cv::Mat& frame){
    detector.detectBalls(frame, table.seg_mask, table.hue_color);
}

void frameHandler::initializeTrackers(const cv::Mat& frame){
    tracker.initializeTrackers(frame, detector.balls);      
}

void frameHandler::updateTrackers(const cv::Mat& frame){
    tracker.updateTrackers(frame);
}

cv::Mat frameHandler::draw_frame(const cv::Mat& frame){
    return table.draw_borders(frame);
}

void frameHandler::project(const cv::Mat& frame){
    projecter.projectBalls(frame, tracker.centers, tracker.trajectories, detector.id_balls, table.corners);
}