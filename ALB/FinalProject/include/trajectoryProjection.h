/*
AUTHOR: Girardello Sofia
*/

#ifndef trajectoryProjection_INCLUDED
  #define trajectoryProjection_INCLUDED

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>

class trajectoryProjecter{

  private: 

    cv::Mat origin_frame;
    cv::Mat roi;
    std::vector<cv::Point2f> balls_centers;

  public:

    std::vector<cv::Point2f> corners;

    //const std::vector<cv::Point2f>& corners
    explicit trajectoryProjecter();
    void projectBalls(const cv::Mat& frame, const std::vector<cv::Point2f>& centers, const std::vector<std::vector<cv::Point2f>>& trajectories, const std::vector<int>& id_balls, const std::vector<cv::Point2f>& corners);

};

#endif