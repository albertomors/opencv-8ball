/*
    AUTHOR: Fresco Eleonora 
    DATE: 2024-07-21
    FILE: trajectoryProjection.h
    DESCRIPTION: Definition of the trajectory projecting class.

    CLASSES:
    - class trajectoryProjecter: Class for projecting the balls' trajectory onto a bird eye view minimap.

    FUNCTIONS:
    - cv::Point2f computeCentroid(const std::vector<cv::Point2f>& points): Computes the centroid of a given set of points.
    - std::vector<cv::Point2f> sortCornersClockwise(std::vector<cv::Point2f>& corners): Sorts corners in clockwise order based on their angle from the centroid.
    - trajectoryProjecter::trajectoryProjecter(): Constructor for the trajectoryProjecter class.
    - void trajectoryProjecter::projectBalls(const cv::Mat& frame, const std::vector<cv::Point2f>& balls, const std::vector<std::vector<cv::Point2f>>& trajectories, const std::vector<int>& id_balls, const std::vector<cv::Point2f>& corners): Projects ball positions and trajectories onto a table minimap and displays the result.

    NOTES:
    - The table minimap image should be placed in the "../res/" directory.
    - The function `projectBalls` overlays the table minimap image onto the bottom-left corner of the input frame.
    - Balls and their trajectories are drawn on the minimap image with colors assigned based on their IDs.
*/

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>

#ifndef trajectoryProjection_INCLUDED
  #define trajectoryProjection_INCLUDED

class trajectoryProjecter{

  private: 

    std::vector<cv::Point2f> balls_centers;

  public:

    explicit trajectoryProjecter();
    cv::Mat projectBalls(const cv::Mat& frame, const std::vector<cv::Point2f>& centers, const std::vector<std::vector<cv::Point2f>>& trajectories, const std::vector<int>& id_balls, std::vector<cv::Point2f>& corners);

};

#endif