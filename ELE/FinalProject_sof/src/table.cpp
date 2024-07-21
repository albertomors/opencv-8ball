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

    this->color = hue_dominant;

    return cv::Scalar(hue_dominant, 127, 127); //mid-value for sat and brightness
}

cv::Mat tableDetector::treshold_mask(const cv::Scalar& color) {
    cv::Mat hsv_img, mask;
    cv::cvtColor(this->origin_frame, hsv_img, cv::COLOR_BGR2HSV);
    cv::GaussianBlur(hsv_img, hsv_img, cv::Size(5, 5), 0, 0);

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
    cv::Mat element = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));
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
    cv::Mat biggest = (labels == curr_largest);
    /* cv::imshow("largest component", biggest);
    cv::waitKey(0); */
    
    return biggest;
}

std::vector<cv::Point> tableDetector::find_contour(const cv::Mat& mask) {
    //find contour of closed area
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    std::vector<cv::Point> contour = contours[0];
    
    return contour;
}

std::vector<cv::Point> tableDetector::get_hull(const std::vector<cv::Point>& contour){
    //convex hull to remove occlusions
    std::vector<cv::Point> hull;
    cv::convexHull(contour, hull);

    return hull;
}

void tableDetector::find_table(const cv::Mat& img){
    this->origin_frame = img.clone();
    cv::Scalar table_color = this->get_dominant_color();
    cv::Mat tresholded_img = this->treshold_mask(table_color);
    cv::Mat mask = this->find_largest_comp(tresholded_img);
    this->contour = this->find_contour(mask);

    this->seg_mask = cv::Mat::zeros(this->origin_frame.size(), CV_8UC1);
    cv::fillPoly(this->seg_mask, this->contour, cv::Scalar(255));

    this->hull = get_hull(this->contour);
}

cv::Mat tableDetector::draw_borders(const cv::Mat& img){
    cv::Mat edited = img.clone();
    cv::polylines(edited, this->contour, true, cv::Scalar(255, 0, 255), 1);
    cv::polylines(edited, this->hull, true, cv::Scalar(0, 255, 255), 2);
    return edited;
}