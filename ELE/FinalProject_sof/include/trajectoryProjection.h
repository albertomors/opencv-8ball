/*
AUTHOR: Girardello Sofia
*/
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>

#ifndef trajectoryProjection_INCLUDED
  #define trajectoryProjection_INCLUDED


class trajectoryProjecter{

  private: 

  cv::Mat origin_frame;
  cv::Mat roi;
  std::vector<cv::Point2f> balls_centers;

  public:

    std::vector<cv::Point2f> corners;

    explicit trajectoryProjecter();

    void findLines(const cv::Mat& current_frame, const cv::Mat& roi);
    void projectBalls(const cv::Mat& frame, const std::vector<cv::Point2f>& balls);

};

#endif