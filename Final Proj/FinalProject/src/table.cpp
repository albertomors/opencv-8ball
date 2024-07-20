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
    this->hue_color = table_color[0];
    cv::Mat tresholded_img = this->treshold_mask(table_color);
    cv::Mat mask = this->find_largest_comp(tresholded_img);

    //find table bgr_color
    cv::Mat masked;
    this->origin_frame.copyTo(masked,mask);
    this->bgr_color = cv::mean(masked);

    this->contour = this->find_contour(mask);
    this->seg_mask = cv::Mat::zeros(this->origin_frame.size(), CV_8UC1);
    cv::fillPoly(this->seg_mask, this->contour, cv::Scalar(255));

    this->hull = get_hull(this->contour);

    //NEW
    this->corners = this->find_corners();
}

cv::Mat tableDetector::draw_borders(const cv::Mat& img){
    cv::Mat edited = img.clone();
    cv::polylines(edited, this->contour, true, cv::Scalar(0, 255, 255), 1); //TODO debug
    cv::polylines(edited, this->hull, true, cv::Scalar(0, 255, 255), 3);

    for (const cv::Point2f& point : this->corners)
        cv::circle(edited, point, 4, cv::Scalar(0, 0, 255), cv::FILLED);
    return edited;
}

//portate dentro da Trajectory.cpp --------------------------------------------

cv::Point2f tableDetector::get_intersection_point(const cv::Vec4i& line1, const cv::Vec4i& line2) {
    float x1 = line1[0], y1 = line1[1], x2 = line1[2], y2 = line1[3];
    float x3 = line2[0], y3 = line2[1], x4 = line2[2], y4 = line2[3];

    float denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    if (denom == 0) {
        return cv::Point2f(-1, -1);  // Parallel lines, no intersection
    }

    float px = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / denom;
    float py = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / denom;

    return cv::Point2f(px, py);
}

std::vector<cv::Point2f> tableDetector::find_corners(){

    cv::Mat ROI = cv::Mat::zeros(this->origin_frame.size(), CV_8UC1);
    cv::fillConvexPoly(ROI, this->hull, cv::Scalar(255));
    /* cv::imshow("ROI", ROI);
    cv::waitKey(0); */

    cv::Mat blurred_ROI;
    cv::GaussianBlur(ROI, blurred_ROI, cv::Size(25, 25), 0);
    cv::Mat canny;
    cv::Canny(blurred_ROI, canny, 20, 50);
    /* cv::imshow("Canny", canny);
    cv::waitKey(0); */

    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(canny, lines, 2, 3*CV_PI/180, 80, 200, 1000);

    /* cv::Mat output;
    this->origin_frame.copyTo(output); */
    for (int i = 0; i < lines.size(); i++) {
        cv::Vec4i l = lines[i];
        //cv::line(output, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
    }

    // Find and highlight intersections
    std::vector<cv::Point2f> intersectionPoints;
    double thresholdDistance = 20.0;

    for (size_t i = 0; i < lines.size(); i++) {
        for (size_t j = i + 1; j < lines.size(); j++) {
            cv::Point2f intersection = get_intersection_point(lines[i], lines[j]);
            if (intersection.x >= 0 && intersection.y >= 0 && intersection.x < this->origin_frame.cols && intersection.y < origin_frame.rows) {
                bool tooClose = false;
                for (const cv::Point2f& point : intersectionPoints) {
                    double dist = std::sqrt(std::pow(intersection.x - point.x, 2) + std::pow(intersection.y - point.y, 2));
                    if (dist < thresholdDistance) {
                        tooClose = true;
                        break;
                    }
                }
                if (!tooClose) {
                    intersectionPoints.push_back(intersection);
                    //cv::circle(edited, point, 4, cv::Scalar(0, 0, 255), cv::FILLED);
                }
            }
        }
    }

    /* cv::imshow("Detected Lines", output);
    cv::waitKey(0); */

    return intersectionPoints;
}