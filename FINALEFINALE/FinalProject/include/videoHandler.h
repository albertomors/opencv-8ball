/*
    AUTHOR: Fresco Eleonora
    DATE: 2024-07-21 
    FILE: videoHandler.h
    DESCRIPTION: Defines the `videoHandler` class for processing video files. 

    CLASS: videoHandler

    METHODS:
    - videoHandler(const std::string& folder_name): Constructor that initializes the `videoHandler` object by setting up paths and loading necessary files based on the provided folder name.
    - void load_files(): Loads segmentation mask and bounding box data for the first and last frames. Handles errors if files cannot be loaded.
    - cv::Mat load_txt_data(...): Reads bounding box data from a text file and stores it in a `cv::Mat` matrix.
    - void process_video(...): Processes the video file frame by frame. Calls `frameHandler` to perform table and ball detection. Displays intermediate results based on the `MIDSTEP_flag` and writes processed frames to an output video file.
    - cv::Mat displayMask(...): Converts and displays segmentation masks using a predefined color map for different classes.
    - cv::Mat plot_bb(...): Draws bounding boxes on the source image using colors based on class labels.

    USAGE:
    - The `videoHandler` class is used to manage the end-to-end process of video frame extraction, processing, and output. It interacts with the `frameHandler` class to detect and analyze objects within the frames, and produces a final video with the results.

    IMPORTANT:
    - Ensure the paths and file names used in `load_files` match the actual dataset structure.
    - The output video and metrics are saved to the `../build/output` directory. Ensure this directory is writable.
    - The `MIDSTEP_flag` allows toggling between visualizing all frames or just the first and last frames for debugging purposes.
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

    void load_files();
    cv::Mat load_txt_data(const std::string& path);

public:

    bool errors;

    explicit videoHandler(const std::string& folder_name);
    
    void process_video(int MIDSTEP_flag);
    cv::Mat plot_bb(const cv::Mat& src, const cv::Mat& bb);
    cv::Mat displayMask(const cv::Mat& mask);

};

#endif