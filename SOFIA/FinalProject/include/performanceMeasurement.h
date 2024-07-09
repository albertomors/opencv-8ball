#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>


#ifndef PERFORMANCEMEASUREMENT_INCLUDED
  #define PERFORMANCEMEASUREMENT_INCLUDED

  void computeMAP(std::vector<cv::Rect>& balls);
  void computeMIOU(cv::Mat& segmentation);

#endif