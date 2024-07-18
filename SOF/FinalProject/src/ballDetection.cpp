/*
AUTHOR: Girardello Sofia 
*/

#include "ballDetection.h"

// Constructor to initialize the frame
ballDetector::ballDetector() {
}


void ballDetector::detectBalls(const cv::Mat& currentFrame, const cv::Mat& ROI, const float field_color, const cv::Mat& seg_mask) {

     // Input validation
    if (currentFrame.size() != ROI.size() || currentFrame.type() != CV_8UC3 || ROI.type() != CV_8UC1) {
        std::cerr << "Error: Invalid input images!" << std::endl;
        return;
    }

    cv::GaussianBlur(currentFrame, currentFrame, cv::Size(5, 5), 0);

    //cv::Mat element1 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(19, 19));
    //cv::morphologyEx(ROI, ROI, cv::MORPH_ERODE, element1);

    cv::Mat table_roi;
    currentFrame.copyTo(table_roi, ROI); // Mask the current frame with ROI
    //cv::imshow("ROI Mask Applied", table_roi);

    // Define the size of the area around the center to compute the average color
    int areaSize = 50; // Adjust this size as needed
    int startX = (currentFrame.cols - areaSize) / 2;
    int startY = (currentFrame.rows - areaSize) / 2;

    cv::Rect centerRect(startX, startY, areaSize, areaSize);
    cv::Mat centerArea = currentFrame(centerRect);

    // Compute the average BGR color
    cv::Scalar avgBGR = cv::mean(centerArea);


    // Convert BGR to HSV
    cv::Mat bgrMat(1, 1, CV_8UC3, avgBGR);
    cv::Mat hsvMat;
    cv::cvtColor(bgrMat, hsvMat, cv::COLOR_BGR2HSV);
    cv::Vec3b hsvColor = hsvMat.at<cv::Vec3b>(0, 0);

    // Convert the frame to HSV
    cv::Mat hsv_img;
    cv::cvtColor(table_roi, hsv_img, cv::COLOR_BGR2HSV);


    // Thresholding based on the average center color to isolate balls
    cv::Mat mask_col;
    cv::inRange(hsv_img, cv::Scalar(hsvColor[0] - 3, 0, 0), cv::Scalar(hsvColor[0] + 7, 200, 200), mask_col);
    cv::imshow("Colour Thresholded Mask", mask_col);

/*
    // Calculate the Euclidean distance from each pixel to the average BGR color
    cv::Mat distanceMat = cv::Mat::zeros(table_roi.size(), CV_32FC1);
    for (int y = 0; y < table_roi.rows; ++y) {
        for (int x = 0; x < table_roi.cols; ++x) {
            cv::Vec3b color = table_roi.at<cv::Vec3b>(y, x);
            float distance = std::sqrt(
                std::pow(color[0] - avgBGR[0], 2) +
                std::pow(color[1] - avgBGR[1], 2) +
                std::pow(color[2] - avgBGR[2], 2)
            );
            distanceMat.at<float>(y, x) = distance;
        }
    }

    // Normalize the distance image to the range [0, 255]
    cv::normalize(distanceMat, distanceMat, 0, 255, cv::NORM_MINMAX);

    // Convert to 8-bit image
    cv::Mat distanceMat8U;
    distanceMat.convertTo(distanceMat8U, CV_8UC1);

    // Apply a threshold to create a mask
    cv::Mat mask_col;
    double thresholdValue = 150; // Adjust this value as needed
    cv::threshold(distanceMat8U, mask_col, thresholdValue, 255, cv::THRESH_BINARY_INV);
*/
    cv::Mat canny_col;
    cv::Canny(mask_col, canny_col, 10, 80);
    cv::imshow("Canny Colour", canny_col);

    // Find contours
    std::vector<std::vector<cv::Point>> contours_thr_col;
    cv::findContours(canny_col, contours_thr_col, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // Create a mask for filling contours
    cv::Mat filled_mask_col = cv::Mat::zeros(canny_col.size(), CV_8UC1);

    // Draw and fill contours
    for (size_t i = 0; i < contours_thr_col.size(); ++i) {
        cv::drawContours(filled_mask_col, contours_thr_col, static_cast<int>(i), cv::Scalar(255), cv::FILLED);
    }
    cv::imshow("Filled mask Colour", filled_mask_col);

    std::vector<cv::Vec3f> circles_col;
    cv::HoughCircles(canny_col, circles_col, cv::HOUGH_GRADIENT, 1.5, mask_col.rows / 20, 90, 30, 2, 13);

    // Draw detected circles on the original table_roi image
    cv::Mat colour_res = table_roi.clone();
    for (size_t i = 0; i < circles_col.size(); i++) {
        cv::Point center(cvRound(circles_col[i][0]), cvRound(circles_col[i][1]));
        int radius = cvRound(circles_col[i][2]);

        // Draw the circle
        cv::circle(colour_res, center, radius, cv::Scalar(0, 255, 0), 2);
    }

    // Show the result with detected circles
    cv::imshow("Detected Balls colour", colour_res);

    // Convert table_roi to grayscale for HoughCircles
    cv::Mat table_roi_gray;
    cv::cvtColor(table_roi, table_roi_gray, cv::COLOR_BGR2GRAY);

    // Ensure the grayscale image is not empty before proceeding
    if (table_roi_gray.empty()) {
        std::cerr << "Error: Grayscale image is empty!" << std::endl;
        return;
    }

    cv::Mat canny;
    cv::Canny(table_roi_gray, canny, 60, 80);
    cv::imshow("Canny", canny);

    std::vector<cv::Vec3f> circles1;
    cv::HoughCircles(canny, circles1, cv::HOUGH_GRADIENT, 1.7, table_roi_gray.rows / 30, 100, 25, 2, 11);

    // Draw detected circles on the original table_roi image
    cv::Mat result2 = table_roi.clone();
    for (size_t i = 0; i < circles1.size(); i++) {
        cv::Point center(cvRound(circles1[i][0]), cvRound(circles1[i][1]));
        int radius = cvRound(circles1[i][2]);

        // Draw the circle
        cv::circle(result2, center, radius, cv::Scalar(0, 255, 0), 2);
    }

    // Show the result with detected circles
    cv::imshow("Detected Balls circles", result2);

    std::vector<cv::Mat> hsv_channels;
    cv::split(hsv_img, hsv_channels);

    // Thresholding based on hue to isolate balls
    cv::Mat mask_thr;
    cv::Mat hue_channel = hsv_channels[0];
    cv::inRange(hsv_img, cv::Scalar(field_color - 2, 0, 0), cv::Scalar(field_color + 2, 255, 255), mask_thr);
    //cv::inRange(hue_channel, field_color-3, field_color+3, mask_thr);
    cv::imshow("Thresholded Mask", mask_thr);

    cv::Canny(seg_mask, canny, 50, 70);

    cv::HoughCircles(canny, circles1, cv::HOUGH_GRADIENT, 1.7, table_roi_gray.rows / 30, 100, 30, 2, 13);

    // Draw detected circles on the original table_roi image
    result2 = table_roi.clone();
    for (size_t i = 0; i < circles1.size(); i++) {
        cv::Point center(cvRound(circles1[i][0]), cvRound(circles1[i][1]));
        int radius = cvRound(circles1[i][2]);

        // Draw the circle
        cv::circle(result2, center, radius, cv::Scalar(0, 255, 0), 2);
    }

    // Show the result with detected circles
    cv::imshow("Detected Balls circles with mask", result2);

    // Find contours
    std::vector<std::vector<cv::Point>> contours_thr;
    cv::findContours(mask_thr, contours_thr, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // Create a mask for filling contours
    cv::Mat filled_mask = cv::Mat::zeros(mask_thr.size(), CV_8UC1);

    // Draw and fill contours
    for (size_t i = 0; i < contours_thr.size(); ++i) {
        cv::drawContours(filled_mask, contours_thr, static_cast<int>(i), cv::Scalar(255), cv::FILLED);
    }

    // Show the filled mask
    //cv::imshow("Filled Contours Mask", filled_mask);

    // Invert the mask
    cv::bitwise_not(mask_thr, mask_thr);

    // Apply mask_thr and filled_mask sequentially to extract balls
    cv::Mat ball_mask = cv::Mat::zeros(table_roi.size(), table_roi.type());
    cv::bitwise_and(table_roi, table_roi, ball_mask, mask_thr);

    //cv::imshow("img", ball_mask);
    //cv::imshow("mask", filled_mask);
    std::cout << "ball_mask size: " << ball_mask.size() << ", type: " << ball_mask.type() << std::endl;
    std::cout << "filled_mask size: " << filled_mask.size() << ", type: " << filled_mask.type() << std::endl;

    cv::cvtColor(ball_mask, ball_mask, cv::COLOR_BGR2GRAY);
    cv::bitwise_and(ball_mask, filled_mask, ball_mask);
    cv::imshow("result", ball_mask);

    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(11, 11));
    cv::morphologyEx(ball_mask, ball_mask, cv::MORPH_ERODE, element);


// Find contours in the ball_mask
std::vector<std::vector<cv::Point>> contours1;
cv::findContours(ball_mask, contours1, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

// Draw rectangles around balls with area larger than a threshold
cv::Mat result1 = table_roi.clone();
const int minBallArea = 100;  // Minimum area threshold for a detected ball
const int maxBallArea = 300;  // Maximum area threshold for a detected ball

std::vector<cv::Rect> ballRects;
std::vector<cv::Point2f> rectCenters;

for (size_t i = 0; i < contours1.size(); ++i) {
        cv::Rect rect = cv::boundingRect(contours1[i]);
        cv::rectangle(result1, rect, cv::Scalar(0, 255, 0), 2); 
        ballRects.push_back(rect);
        rectCenters.push_back(cv::Point2f(rect.tl().x, rect.tl().y) + 0.5 * cv::Point2f(rect.width, rect.height));
}
//cv::imshow("Before Detected Balls", result1);

for (size_t i = 0; i < contours1.size(); ++i) {
    double area = cv::contourArea(contours1[i]);
    if (area >= minBallArea && area <= maxBallArea) {
        cv::Rect rect = cv::boundingRect(contours1[i]);
        cv::rectangle(result1, rect, cv::Scalar(0, 255, 0), 2); 
        ballRects.push_back(rect);
        rectCenters.push_back(cv::Point2f(rect.tl().x, rect.tl().y) + 0.5 * cv::Point2f(rect.width, rect.height));
    }
}
this->centers = rectCenters;
this->balls = ballRects;

cv::imshow("Detected Balls", result1);
cv::waitKey(0);

    
    /*// Convert the table_roi to grayscale and apply Gaussian blur and Canny edge detection
    cv::Mat img_gray;
    cv::cvtColor(table_roi, img_gray, cv::COLOR_BGR2GRAY);
    cv::Mat blurred_img;
    cv::GaussianBlur(img_gray, blurred_img, cv::Size(3, 3), 0);
    cv::Mat edges;
    cv::Canny(blurred_img, edges, 20, 50);
    cv::imshow("Canny Edges", edges);

    // Use Hough Circle Transform to detect circles
    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(edges, circles, cv::HOUGH_GRADIENT, 1.6, edges.rows / 35, 40, 25, 5, 12);

    // Draw detected circles and analyze each circle
    cv::Mat result = currentFrame.clone();
    std::vector<cv::Point2f> centers;

    for (size_t i = 0; i < circles.size(); i++) {
        cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int radius = cvRound(circles[i][2]);

        // Check if the circle center is close enough to any of the rectCenters
        for (size_t j = 0; j < rectCenters.size(); ++j) {
            cv::Point2f center_f(center.x, center.y);
            float dist = cv::norm(center_f - rectCenters[j]);
            if (dist < 10) {  // Adjust this threshold as needed
                // Ensure the radius is within a reasonable range
                if (radius > 5 && radius < 12) {
                    // Create a mask for the detected circle
                    cv::Mat circleMask = cv::Mat::zeros(currentFrame.size(), CV_8UC1);
                    cv::circle(circleMask, center, radius, cv::Scalar(255), -1);

                    // Extract the ROI from the original frame
                    cv::Mat ballROI;
                    currentFrame.copyTo(ballROI, circleMask);

                    // Analyze the ROI to determine if it is striped or solid
                    bool isStriped = analyzeBallPattern(ballROI, center, radius);

                    // Draw the detected circle and label it
                    cv::Scalar color = isStriped ? cv::Scalar(255, 0, 0) : cv::Scalar(0, 255, 0);  // Blue for striped, Green for solid
                    cv::circle(result, center, radius, color, 3, 8, 0);
                    centers.push_back(cv::Point2f(center.x, center.y));
                }
                break;  
            }
        }
    }

    this->centers = centers;
    // Show the result
    cv::imshow("Classification Result", result);
    cv::waitKey(0);
    */
}


bool ballDetector::analyzeBallPattern(const cv::Mat& ballROI, const cv::Point& center, int radius) {
    // Convert the ROI to grayscale
    cv::Mat gray;
    cv::cvtColor(ballROI, gray, cv::COLOR_BGR2GRAY);

    // Threshold the grayscale image to create a binary image
    cv::Mat binary;
    cv::threshold(gray, binary, 200, 255, cv::THRESH_BINARY); 

    // Create a circular mask to focus only on the ball
    cv::Mat circleMask = cv::Mat::zeros(binary.size(), CV_8UC1);
    cv::circle(circleMask, center, radius, cv::Scalar(255), -1);

    // Apply the circular mask to the binary image
    cv::Mat maskedBinary;
    binary.copyTo(maskedBinary, circleMask);

    // Calculate the percentage of white pixels
    int whitePixels = cv::countNonZero(maskedBinary);
    int totalPixels = cv::countNonZero(circleMask);
    double whitePercentage = (double)whitePixels / totalPixels * 100;

    // Determine if the ball is striped or solid
    return whitePercentage > 5.0;
}


