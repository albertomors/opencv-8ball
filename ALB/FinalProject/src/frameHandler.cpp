/*
AUTHOR: Morselli Alberto 
FILE: frameHandler.cpp
*/

#include "frameHandler.h"
#include "table.h"

frameHandler::frameHandler(){
    this->table = tableDetector();
}

void frameHandler::detect_table(const cv::Mat& frame){
    this->table.find_table(frame);
}

cv::Mat frameHandler::draw_frame(const cv::Mat& frame){
    return this->table.draw_borders(frame);
}