/*
AUTHOR: Girardello Sofia 
*/

#include "ballDetection.h"

// Constructor to initialize the frame
ballDetector::ballDetector() {
}


void ballDetector::detectBalls(const cv::Mat& currentFrame, const cv::Mat& ROI, const float field_color) {
    // Input validation
    if (currentFrame.size() != ROI.size() || currentFrame.type() != CV_8UC3 || ROI.type() != CV_8UC1) {
        std::cerr << "Error: Invalid input images!" << std::endl;
        return;
    }

    cv::Mat table_roi;
    currentFrame.copyTo(table_roi, ROI); // Mask the current frame with ROI
    cv::imshow("ROI Mask Applied", table_roi);

    // Convert the frame to HSV
    cv::Mat hsv_img;
    cv::cvtColor(table_roi, hsv_img, cv::COLOR_BGR2HSV);

    // Thresholding based on hue to isolate balls
    cv::Mat mask_thr;
    cv::inRange(hsv_img, cv::Scalar(field_color - 4, 100, 100), cv::Scalar(field_color + 4, 255, 255), mask_thr);
    cv::imshow("Thresholded Mask", mask_thr);

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
    cv::imshow("Filled Contours Mask", filled_mask);
    cv::waitKey(0);

    // Invert the mask
    cv::bitwise_not(mask_thr, mask_thr);

    // Apply mask_thr and filled_mask sequentially to extract balls
    cv::Mat ball_mask = cv::Mat::zeros(table_roi.size(), table_roi.type());
    cv::bitwise_and(table_roi, table_roi, ball_mask, mask_thr);

    cv::imshow("img", ball_mask);
    cv::imshow("mask", filled_mask);
    std::cout << "ball_mask size: " << ball_mask.size() << ", type: " << ball_mask.type() << std::endl;
    std::cout << "filled_mask size: " << filled_mask.size() << ", type: " << filled_mask.type() << std::endl;

    cv::cvtColor(ball_mask, ball_mask, cv::COLOR_BGR2GRAY);
    cv::bitwise_and(ball_mask, filled_mask, ball_mask);
    cv::imshow("result", ball_mask);

    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::morphologyEx(ball_mask, ball_mask, cv::MORPH_ERODE, element);

/*
    std::vector<cv::Vec3f> circles1;
    cv::HoughCircles(ball_mask, circles1, cv::HOUGH_GRADIENT, 2, ball_mask.rows / 30, 100, 30, 5, 15);

// Draw detected circles on the original table_roi image
cv::Mat result1 = table_roi.clone();
for (size_t i = 0; i < circles1.size(); i++) {
    cv::Point center(cvRound(circles1[i][0]), cvRound(circles1[i][1]));
    int radius = cvRound(circles1[i][2]);

    // Draw the circle
    cv::circle(result1, center, radius, cv::Scalar(0, 255, 0), 2);
}

// Show the result with detected circles
cv::imshow("Detected Balls", result1);
cv::waitKey(0);
*/

// Find contours in the ball_mask
std::vector<std::vector<cv::Point>> contours1;
cv::findContours(ball_mask, contours1, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

// Draw rectangles around balls with area larger than a threshold
cv::Mat result1 = table_roi.clone();
const int minBallArea = 60;  // Minimum area threshold for a detected ball
const int maxBallArea = 500;  // Maximum area threshold for a detected ball

std::vector<cv::Rect> ballRects;
std::vector<cv::Point2f> rectCenters;
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

cv::imshow("Detected Balls", result1);

    
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


