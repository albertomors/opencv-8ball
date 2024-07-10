/*
AUTHOR: Morselli Alberto 
FILE: videoHandler.h

-given the folder path extract the frames and pass them to frameHandler
-show frame by frame results if flag is true, otherwise just first and final
-receive frames from frameHandler to build the final video
-call metrics at the end
*/

#ifndef VIDEOHANDLER_INCLUDED
#define VIDEOHANDLER_INCLUDED

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>

#include <fstream>
#include <opencv2/core/utils/filesystem.hpp>
#include <filesystem>

class videoHandler{

private:

    std::string folder_name;

    //seg-masks
    cv::Mat ffirst_mask;        //groundtruth
    cv::Mat ffirst_ret_mask;    //computed
    cv::Mat flast_mask;
    cv::Mat flast_ret_mask;

    //b-boxes
    cv::Mat ffirst_bb;
    cv::Mat ffirst_ret_bb;
    cv::Mat flast_bb;
    cv::Mat flast_ret_bb;

    //-- member functions

    void load_files();
    cv::Mat load_txt_data(const std::string& path);

public:

    bool errors;

    explicit videoHandler(const std::string& folder_name);
    void process_video(int MIDSTEP_flag);                         //process video and call frameHandler iter by iter

};

cv::Mat displayMask(const cv::Mat& mask);

#endif