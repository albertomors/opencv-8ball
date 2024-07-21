/*
    AUTHOR: Morselli Alberto
    DATE: 2024-07-21
    FILE: table.h
    DESCRIPTION: Definition of the tableDetector class.

    CLASS: tableDetector

    METHODS:
    - tableDetector::tableDetector(): Default constructor for initializing the `tableDetector` object.
    - cv::Scalar get_dominant_color(): Calculates the dominant color in the image by analyzing the hue channel in HSV color space. The dominant color is returned with mid-range saturation and brightness.
    - cv::Mat treshold_mask(...): Creates a binary mask of the image where the pixels fall within the specified color range. Applies Gaussian blur to smooth the image before thresholding.
    - cv::Mat find_largest_comp(...): Identifies the largest connected component in the binary mask. Performs morphological closing to remove small holes and noise.
    - std::vector<cv::Point> find_contour(...): Finds the contour of the largest connected component. Uses `findContours` to extract the contour points.
    - std::vector<cv::Point> get_hull(...): Computes the convex hull of the contour.
    - void find_table(...): Main method for detecting the table. It calculates the dominant color, thresholds the image, finds the largest component, determines the table color, detects contours, computes the convex hull, and finds table corners.
    - cv::Mat draw_borders(...): Draws the detected table borders and corners on the image for visualization.
    - cv::Point2f get_intersection_point(...): Computes the intersection point of two lines defined by their endpoints. Handles cases where lines are parallel.
    - std::vector<cv::Point2f> tableDetector::find_corners(): Detects and returns corners of the table by finding intersections of detected lines. 

    NOTES:
    - The color thresholding is manually tuned for the table's expected color in the HSV color space.
    - The `find_corners` method uses the Hough Line Transform to detect lines and their intersections, which are then used to identify table corners.
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