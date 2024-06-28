#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>


#ifndef VIDEOPROCESSING_INCLUDED
  #define VIDEOPROCESSING_INCLUDED

  class videoProcessor{

    /*
    Class videoEditor :
    */

    private: 

    std::string pathToVideos;
    std::string inputVideo;
    cv::Mat firstFrame;
    cv::Mat lastFrame;
    cv::VideoWriter writer;

    public:

    explicit videoProcessor(const std::string folderName);

    void processVideo();

  };
#endif