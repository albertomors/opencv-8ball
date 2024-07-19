/*
AUTHOR: Morselli Alberto 
FILE: table.h

-TODO description
*/

#ifndef TABLE_INCLUDE
#define TABLE_INCLUDE

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>

class tableDetector{

  private:

      cv::Mat origin_frame;
      std::vector<cv::Point> borders;

      cv::Scalar get_dominant_color();
      cv::Mat treshold_mask(const cv::Scalar& color);
      cv::Mat find_largest_comp(const cv::Mat& mask);
      std::vector<cv::Point> find_borders(const cv::Mat& mask);

  public:

      cv::Mat seg_mask;
      cv::Mat ROI;

      explicit tableDetector();
      void find_table(const cv::Mat& img);
      cv::Mat draw_borders(const cv::Mat& img);
};

#endif