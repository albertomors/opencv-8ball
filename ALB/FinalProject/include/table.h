#ifndef TABLE_INCLUDE
#define TABLE_INCLUDE

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>

cv::Scalar get_dominant_color(const cv::Mat& img);
cv::Mat treshold_mask(const cv::Mat& img, const cv::Scalar& color);
cv::Mat find_largest_comp(const cv::Mat& mask);
std::vector<cv::Point> find_borders(const cv::Mat& mask);
cv::Mat highlight_borders(const cv::Mat& img);

#endif