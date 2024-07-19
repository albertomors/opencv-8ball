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

  double compute_mAP(const cv::Mat& pred_bb, const cv::Mat& true_bb);
  double compute_IoU(const cv::Rect& r1, const cv::Rect& r2);
  std::vector<cv::Point2f> get_PR_table(const cv::Mat& pred_bb, const cv::Mat& true_bb, int pred_class);
  std::vector<cv::Point2f> refine_PR_table(std::vector<cv::Point2f>& points);
  
  double compute_mIoU(const std::vector<std::pair<cv::Mat, cv::Mat>>& videoSegMasks, int numClasses);


#endif