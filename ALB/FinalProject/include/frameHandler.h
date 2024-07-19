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

class frameHandler{

private:

    tableDetector table;
    //detector
    //tracker

public:

    explicit frameHandler();

    void detect_table(const cv::Mat& frame);
    cv::Mat draw_frame(const cv::Mat& frame);
};

#endif