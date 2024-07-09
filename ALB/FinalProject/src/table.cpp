#include <opencv2/opencv.hpp>
#include <iostream>

cv::Scalar get_dominant_color(const cv::Mat& img) {
    cv::Mat hsv_img;
    cv::cvtColor(img, hsv_img, cv::COLOR_BGR2HSV);

    int h_bins = 128;
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

cv::Mat treshold_mask(const cv::Mat& img, const cv::Scalar& color) {
    cv::Mat hsv_img, mask;
    cv::cvtColor(img, hsv_img, cv::COLOR_BGR2HSV);

    //accepted ranges - HANDTUNED
    cv::Scalar lower_bound(color[0] - 5, 50, 50);
    cv::Scalar upper_bound(color[0] + 5, 250, 250);

    cv::inRange(hsv_img, lower_bound, upper_bound, mask); //apply treshold
    cv::imshow("mask", mask);
    cv::waitKey(0);

    return mask;
}

cv::Mat find_largest_comp(const cv::Mat& mask) {
    cv::Mat labels, stats, centroids;

    cv::imshow("before morph", mask);
    cv::waitKey(0);

    //closing pre-processing
    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, element);
    
    cv::imshow("after morph", mask);
    cv::waitKey(0);

    int  num = connectedComponentsWithStats(mask, labels, stats, centroids);

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
    cv::Mat largest_mask = (labels == curr_largest);
    cv::imshow("largest component", largest_mask);
    cv::waitKey(0);

    return largest_mask;
}

cv::Point computeIntersect(cv::Vec4i line1, cv::Vec4i line2) {
    cv::Point intersection;
    float denom = ((float)line1[0] - line1[2]) * ((float)line2[1] - line2[3]) -
                  ((float)line1[1] - line1[3]) * ((float)line2[0] - line2[2]);

    if (denom != 0) {
        intersection.x = ((float)line1[0] * line1[3] - (float)line1[1] * line1[2]) * ((float)line2[0] - line2[2]) -
                         ((float)line1[0] - line1[2]) * ((float)line2[0] * line2[3] - (float)line2[1] * line2[2]) / denom;

        intersection.y = ((float)line1[0] * line1[3] - (float)line1[1] * line1[2]) * ((float)line2[1] - line2[3]) -
                         ((float)line1[1] - line1[3]) * ((float)line2[0] * line2[3] - (float)line2[1] * line2[2]) / denom;
    }
    intersection.x = static_cast<int>(intersection.x);
    intersection.y = static_cast<int>(intersection.y);
    return intersection;
}

std::vector<cv::Point> find_borders(const cv::Mat& mask) {
    //find contour of closed area
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    std::vector<cv::Point> contour = contours[0];

    //convex hull to handle occlusions
    std::vector<cv::Point> hull;
    cv::convexHull(contour, hull);

    cv::Mat contour_img = mask.clone();
    cv::cvtColor(contour_img, contour_img, cv::COLOR_GRAY2BGR);
    cv::drawContours(contour_img, contours, 0, cv::Scalar(0, 255, 0), 2);
    cv::polylines(contour_img, hull, true, cv::Scalar(0, 0, 255), 2);
    cv::imshow("contours", contour_img);
    cv::waitKey(0);

    return hull;
}

int main(int argc, char** argv) {
    std::string folder_name = argv[1];
    cv::Mat img = cv::imread("../Dataset/" + folder_name + "/frames/frame_first.png");

    cv::Scalar table_color = get_dominant_color(img);
    cv::Mat table_mask = treshold_mask(img, table_color);
    cv::Mat table = find_largest_comp(table_mask);
    std::vector<cv::Point> borders = find_borders(table);

    cv::polylines(img, borders, true, cv::Scalar(255, 0, 0), 2);
    cv::imshow("Billiard Table", img);
    cv::waitKey(0);

    return 0;
}