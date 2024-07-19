/*
AUTHOR: Morselli Alberto 
FILE: metrics.h

-called by videoHandler
-compute metrics, show and save them
*/

#ifndef METRICS_INCLUDED
  #define METRICS_INCLUDED

  #include <opencv2/highgui.hpp>
  #include <opencv2/imgproc.hpp>
  #include <opencv2/opencv.hpp>
  #include <iostream>

  void compute_mAP(std::vector<cv::Rect>& balls);
  //void compute_mIoU(cv::Mat& seg_mask);
  double compute_mIoU(const std::vector<std::pair<cv::Mat, cv::Mat>>& videoSegMasks, int numClasses); //Ele


#endif