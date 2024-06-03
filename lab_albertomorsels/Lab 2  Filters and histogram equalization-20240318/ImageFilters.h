//ImageFilters.h

#ifndef IMAGEFILTERS_H
#define IMAGEFILTERS_H

#include <opencv2/core.hpp>

cv::Mat maxFilter(const cv::Mat& src, int kernel_size);
cv::Mat minFilter(const cv::Mat& src, int kernel_size);
cv::Mat medFilter(const cv::Mat& src, int kernel_size);

#endif //IMAGEFILTERS_H