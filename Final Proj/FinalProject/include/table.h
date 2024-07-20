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
      cv::Mat table_roi;

      cv::Scalar get_dominant_color();
      cv::Mat treshold_mask(const cv::Scalar& color);
      cv::Mat find_largest_comp(const cv::Mat& mask);
      std::vector<cv::Point> find_contour(const cv::Mat& mask);
      std::vector<cv::Point> get_hull(const std::vector<cv::Point>&);

      std::vector<cv::Point2f> find_corners(); //NEW
      cv::Point2f get_intersection_point(const cv::Vec4i& line1, const cv::Vec4i& line2);

  public:

      std::vector<cv::Point> contour;   //internal excluding occlusion
      cv::Mat seg_mask;
      std::vector<cv::Point> hull;      //external including them
      std::vector<cv::Point2f> corners; //NEW

      float hue_color;
      cv::Scalar bgr_color;

      explicit tableDetector();
      void find_table(const cv::Mat& img);
      cv::Mat draw_borders(const cv::Mat& img);
};

#endif