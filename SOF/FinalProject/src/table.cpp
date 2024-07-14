/*
AUTHOR: Morselli Alberto 
FILE: table.cpp

-TODO description
*/

#include "table.h"

tableDetector::tableDetector(){}

cv::Scalar tableDetector::get_dominant_color() {
    
    cv::Mat hsv_img;
    cv::cvtColor(this->origin_frame, hsv_img, cv::COLOR_BGR2HSV);

    int h_bins = 64;
    int bins[] = {h_bins};
    float hue_range[] = {0, 180};
    const float* range[] = {hue_range};
    int channels[] = {0}; //look only on hue-channel
    cv::Mat hist;
    cv::calcHist(&hsv_img, 1, channels, cv::Mat(), hist, 1, bins, range, true, false);

    //find maximum value
    double max_val = 0;
    int max_val_idx = 0;
    cv::minMaxIdx(hist, 0, &max_val, 0, &max_val_idx);
    float hue_dominant = (max_val_idx * 180.0) / h_bins; //convert back to hue-value

    cv::Scalar avgBrightness = cv::mean(hsv_img);
    // Extract the V channel and compute the median brightness
    std::vector<uchar> vValues;
    for (int i = 0; i < hsv_img.rows; i++) {
        for (int j = 0; j < hsv_img.cols; j++) {
            vValues.push_back(hsv_img.at<cv::Vec3b>(i, j)[2]);
        }
    }
    std::nth_element(vValues.begin(), vValues.begin() + vValues.size() / 2, vValues.end());
    float medianV = vValues[vValues.size() / 2];

    this->field_color = hue_dominant;

    return cv::Scalar(hue_dominant, 127, medianV); //mid-value for sat and brightness
}

cv::Mat tableDetector::threshold_mask(const cv::Scalar& color) {
    
    cv::Mat hsv_img, mask;
    cv::cvtColor(this->origin_frame, hsv_img, cv::COLOR_BGR2HSV);

    //accepted ranges - HANDTUNED
    cv::Scalar lower_bound(color[0] - 10, 100, 60);
    cv::Scalar upper_bound(color[0] + 10, 250, 250);

    cv::inRange(hsv_img, lower_bound, upper_bound, mask); //apply treshold
    /* cv::imshow("mask", mask);
    cv::waitKey(0); */

    return mask;
}

cv::Mat tableDetector::find_largest_comp(const cv::Mat& mask) {
    cv::Mat labels, stats, centroids;

    /* cv::imshow("before morph", mask);
    cv::waitKey(0); */

    //closing pre-processing
    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, element);
    
    /* cv::imshow("after morph", mask);
    cv::waitKey(0); */

    int num = connectedComponentsWithStats(mask, labels, stats, centroids);

    //find the largest component excluding background
    int curr_largest = 1;
    int curr_maxval = stats.at<int>(1, cv::CC_STAT_AREA);
    for (int i=2; i<num; i++) {
        int area = stats.at<int>(i, cv::CC_STAT_AREA);
        if (area > curr_maxval) {
            curr_largest = i;
            curr_maxval = area;
    }}

    //create the mask
    cv::Mat seg_mask = (labels == curr_largest);
    /* cv::imshow("largest component", largest_mask);
    cv::waitKey(0); */
    
    return seg_mask;
}

std::vector<cv::Point> tableDetector::find_borders(const cv::Mat& mask) {
    //find contour of closed area
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    std::vector<cv::Point> contour = contours[0];

    //convex hull to handle occlusions
    std::vector<cv::Point> hull;
    cv::convexHull(contour, hull);

    /* cv::Mat contour_img = mask.clone();
    cv::cvtColor(contour_img, contour_img, cv::COLOR_GRAY2BGR);
    cv::drawContours(contour_img, contours, 0, cv::Scalar(0, 255, 0), 2);
    cv::polylines(contour_img, hull, true, cv::Scalar(0, 0, 255), 2);
    cv::imshow("contours", contour_img);
    cv::waitKey(0); */

    return hull;
}

std::vector<cv::Point> tableDetector::find_internal_borders(const cv::Mat& mask) {
    cv::Mat labels, stats, centroids;

    //closing pre-processing
    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, element);
    
    int num = connectedComponentsWithStats(mask, labels, stats, centroids);

    //find the second largest component excluding background and largest
    int largest_area = -1;
    int largest_idx = -1;
    int second_largest_area = -1;
    int second_largest_idx = -1;

    for (int i = 1; i < num; i++) {
        int area = stats.at<int>(i, cv::CC_STAT_AREA);
        if (area > largest_area) {
            second_largest_area = largest_area;
            second_largest_idx = largest_idx;
            largest_area = area;
            largest_idx = i;
        } else if (area > second_largest_area) {
            second_largest_area = area;
            second_largest_idx = i;
        }
    }

    if (second_largest_idx == -1) {
        return std::vector<cv::Point>();
    }

    cv::Mat seg_mask = (labels == second_largest_idx);
    return find_borders(seg_mask); // Return borders of the second largest component
}

void tableDetector::find_table(const cv::Mat& img){
    this->origin_frame = img.clone();
    cv::Scalar table_color = this->get_dominant_color();
    cv::Mat tresholded = this->threshold_mask(table_color);
    this->seg_mask = this->find_largest_comp(tresholded);
    this->borders = this->find_borders(seg_mask);
    this->internal_borders = this->find_internal_borders(seg_mask);

    this->ROI = cv::Mat::zeros(this->origin_frame.size(), CV_8UC1);
    cv::Mat hull_mat(this->borders);
    cv::fillConvexPoly(this->ROI, hull_mat, cv::Scalar(255));
}

cv::Mat tableDetector::draw_borders(const cv::Mat& img){
    cv::Mat edited = img.clone();
    cv::polylines(edited, this->borders, true, cv::Scalar(0, 255, 255), 2);
    cv::polylines(edited, this->internal_borders, true, cv::Scalar(0, 0, 0), 2);
    return edited;
}

