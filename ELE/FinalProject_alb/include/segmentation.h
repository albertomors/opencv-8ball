#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>


#ifndef SEGMENTATION_INCLUDED
  #define SEGMENTATION_INCLUDED

  class segmenter{

    /*
    Class segmenter :
    */

    private: 

    cv::Mat frame;
    cv::Mat segmentation;
    std::vector<cv::Point> field;
    std::vector<cv::Rect> balls;
    bool draw;
    
    public:

    explicit segmenter(const cv::Mat currentFrame, std::vector<cv::Point> field, std::vector<cv::Rect> balls);

    void segment();

    void drawSegmentation();

  };
#endif