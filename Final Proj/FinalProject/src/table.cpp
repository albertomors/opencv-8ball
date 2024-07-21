/*
    AUTHOR: Morselli Alberto
    DATE: 2024-07-21
    FILE: table.cpp
    DESCRIPTION: Contains the implementation of the `tableDetector` class. This class provides functionality for detecting and processing the table within an image, including color detection, thresholding, finding contours, and detecting table corners.

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

#include "table.h"

tableDetector::tableDetector(){
}


cv::Scalar tableDetector::get_dominant_color() {

    // Translate the image in HSV color scale
    cv::Mat hsv_img;
    cv::cvtColor(this->origin_frame, hsv_img, cv::COLOR_BGR2HSV);

    int h_bins = 64;
    int bins[] = {h_bins};
    float hue_range[] = {0, 180};
    const float* range[] = {hue_range};
    int channels[] = {0}; //look only on hue-channel
    cv::Mat hist;
    cv::calcHist(&hsv_img, 1, channels, cv::Mat(), hist, 1, bins, range, true, false);

    // Find maximum value of the hue channel
    double max_val = 0;
    int max_val_idx = 0;
    cv::minMaxIdx(hist, 0, &max_val, 0, &max_val_idx);
    float hue_dominant = (max_val_idx * 180.0) / h_bins; 

    return cv::Scalar(hue_dominant, 127, 127); // using mid-value for saturation and brightness
}


cv::Mat tableDetector::treshold_mask(const cv::Scalar& color) {

    // Translate the image in HSV color scale and apply a gaussian smoothing
    cv::Mat hsv_img, mask;
    cv::cvtColor(this->origin_frame, hsv_img, cv::COLOR_BGR2HSV);
    cv::GaussianBlur(hsv_img, hsv_img, cv::Size(5, 5), 0, 0);

    // Define the accepted ranges (handtuned)
    cv::Scalar lower_bound(color[0] - 10, 100, 60);
    cv::Scalar upper_bound(color[0] + 10, 250, 250);

    // Apply treshold
    cv::inRange(hsv_img, lower_bound, upper_bound, mask);

    return mask;
}


cv::Mat tableDetector::find_largest_comp(const cv::Mat& mask) {

    cv::Mat labels, stats, centroids;

    // Closing (pre-processing)
    cv::Mat element = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, element);

    int num = connectedComponentsWithStats(mask, labels, stats, centroids);

    // Find the largest component excluding background
    int curr_largest = 1;
    int curr_maxval = stats.at<int>(1, cv::CC_STAT_AREA);
    for (int i=2; i<num; i++) {
        int area = stats.at<int>(i, cv::CC_STAT_AREA);
        if (area > curr_maxval) {
            curr_largest = i;
            curr_maxval = area;
        }
    }

    // Create the mask
    cv::Mat biggest = (labels == curr_largest);
    
    return biggest;
}


std::vector<cv::Point> tableDetector::find_contour(const cv::Mat& mask) {

    // Find contour of closed area
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    std::vector<cv::Point> contour = contours[0];
    
    return contour;
}


std::vector<cv::Point> tableDetector::get_hull(const std::vector<cv::Point>& contour){

    //Convex hull to remove occlusions
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

    // Find table bgr_color
    cv::Mat masked;
    this->origin_frame.copyTo(masked,mask);
    this->bgr_color = cv::mean(masked);

    this->contour = this->find_contour(mask);
    this->seg_mask = cv::Mat::zeros(this->origin_frame.size(), CV_8UC1);
    cv::fillPoly(this->seg_mask, this->contour, cv::Scalar(255));

    this->hull = get_hull(this->contour);
    this->corners = this->find_corners();

}


cv::Mat tableDetector::draw_borders(const cv::Mat& img){

    cv::Mat edited = img.clone();
    cv::polylines(edited, this->contour, true, cv::Scalar(0, 255, 255), 1); 
    cv::polylines(edited, this->hull, true, cv::Scalar(0, 255, 255), 3);

    for (const cv::Point2f& point : this->corners)
        cv::circle(edited, point, 4, cv::Scalar(0, 0, 255), cv::FILLED);
    return edited;

}


cv::Point2f tableDetector::get_intersection_point(const cv::Vec4i& line1, const cv::Vec4i& line2) {

    // Ectract the coordinates of the points defining the lines
    float x1 = line1[0], y1 = line1[1], x2 = line1[2], y2 = line1[3];
    float x3 = line2[0], y3 = line2[1], x4 = line2[2], y4 = line2[3];

    float denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    if (denom == 0) {
        return cv::Point2f(-1, -1);  // Parallel lines, no intersection
    }

    // Equations to find the intersection point between two lines
    float px = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / denom;
    float py = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / denom;

    return cv::Point2f(px, py);
}

std::vector<cv::Point2f> tableDetector::find_corners(){

    cv::Mat ROI = cv::Mat::zeros(this->origin_frame.size(), CV_8UC1);
    cv::fillConvexPoly(ROI, this->hull, cv::Scalar(255));

    cv::Mat blurred_ROI;
    cv::GaussianBlur(ROI, blurred_ROI, cv::Size(25, 25), 0);
    cv::Mat canny;
    cv::Canny(blurred_ROI, canny, 20, 50);

    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(canny, lines, 2, 3*CV_PI/180, 80, 200, 1000);

    /* --Debug
    cv::Mat output;
    this->origin_frame.copyTo(output); 
    for (int i = 0; i < lines.size(); i++) {
        cv::Vec4i l = lines[i];
        cv::line(output, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
    }*/

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
                    //--Debug  cv::circle(output, point, 4, cv::Scalar(0, 0, 255), cv::FILLED);
                }
            }
        }
    }

    /*--Debug
    cv::imshow("Detected Lines", output);
    cv::waitKey(0);*/

    return intersectionPoints;
}