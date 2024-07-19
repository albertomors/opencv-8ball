/*
AUTHOR: Girardello Sofia 
*/

#include "ballDetection.h"

// Constructor to initialize the frame
ballDetector::ballDetector() {
}


void ballDetector::detectBalls(const cv::Mat& currentFrame, const cv::Mat& ROI, const float color) {

    // Input validation
    if (currentFrame.size() != ROI.size() || currentFrame.type() != CV_8UC3 || ROI.type() != CV_8UC1) {
        std::cerr << "Error: Invalid input images!" << std::endl;
        return;
    }

    //cv::imshow("ROI", ROI);

    cv::Mat table_roi;
    currentFrame.copyTo(table_roi, ROI); // Mask the current frame with ROI
    //cv::imshow("ROI Mask Applied", table_roi);
    cv::waitKey(1);
    
    // Convert the frame to HSV
    cv::Mat hsv_img;
    cv::cvtColor(table_roi, hsv_img, cv::COLOR_BGR2HSV);

    // Thresholding based on hue to isolate balls
    cv::Mat mask_col;
    cv::inRange(hsv_img, cv::Scalar(color - 5, 0, 0), cv::Scalar(color + 5, 255, 255), mask_col);
    //cv::inRange(hue_channel, field_color-3, field_color+3, mask_thr);
    //cv::imshow("Thresholded Mask", mask_col);

    cv::Mat canny_col;
    cv::Canny(mask_col, canny_col, 30, 50);
    //cv::imshow("Canny Colour", canny_col);
    cv::waitKey(1);

    std::vector<cv::Vec3f> circles_col;
    cv::HoughCircles(mask_col, circles_col, cv::HOUGH_GRADIENT, 1.9, mask_col.rows / 30, 100, 15, 4, 13);

    // Draw detected circles on the original table_roi image
    cv::Mat colour_res = table_roi.clone();

    
    for (size_t i = 0; i < circles_col.size(); i++) {
        cv::Point center(cvRound(circles_col[i][0]), cvRound(circles_col[i][1]));
        int radius = cvRound(circles_col[i][2]);

        // Draw the circle
        cv::circle(colour_res, center, radius, cv::Scalar(0, 255, 0), 2);
    }

    // Show the result with detected circles
    //cv::imshow("Detected Balls colour before", colour_res);

    // Draw detected circles on the original table_roi image
    cv::Mat colour_res1 = table_roi.clone();
    

    for (size_t i = 0; i < circles_col.size(); i++) {
        cv::Point center(cvRound(circles_col[i][0]), cvRound(circles_col[i][1]));
        int radius = cvRound(circles_col[i][2]);

        // Create a mask for the detected circle
        cv::Mat circleMask = cv::Mat::zeros(mask_col.size(), CV_8UC1);
        cv::circle(circleMask, center, radius, cv::Scalar(255), -1);

        // Check the area inside the circle in both the segmentation mask and the thresholded mask
        cv::Mat segCircle, threshCircle;
        cv::bitwise_and(ROI, ROI, segCircle, circleMask);
        cv::bitwise_and(~mask_col, ~mask_col, threshCircle, circleMask);
        /* cv::imshow("circleMask", circleMask);
        cv::imshow("~mask_col", ~mask_col);
        cv::imshow("tresh circle", threshCircle); */
        
        // Calculate the area of the white pixels in the segmentation mask and black pixels in the thresholded mask
        double area = cv::countNonZero(circleMask);
        double whiteSegArea = cv::countNonZero(segCircle);
        double blackThreshArea = cv::countNonZero(threshCircle);
        std::cout << "black  "<< blackThreshArea << "white  "<< whiteSegArea;

        if (whiteSegArea/area > 0.8 && blackThreshArea/area > 0.6 && blackThreshArea/whiteSegArea > 0.6) { 
            // Draw the circle
            cv::circle(colour_res1, center, radius, cv::Scalar(0, 255, 0), 1);
        }
    }

    // Show the result with detected circles
    //cv::imshow("Detected Balls colour", colour_res1);

    cv::Mat gray;
    cv::cvtColor(table_roi, gray, cv::COLOR_BGR2GRAY);

    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(gray, circles, cv::HOUGH_GRADIENT, 0.5, mask_col.rows / 30, 20, 10, 4, 13);

    // Draw detected circles on the original table_roi image
    cv::Mat res1 = table_roi.clone();
    for (size_t i = 0; i < circles.size(); i++) {
        cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int radius = cvRound(circles[i][2]);

        // Draw the circle
        cv::circle(res1, center, radius, cv::Scalar(0, 255, 0), 2);
    }
    cv::imshow("Detected Balls before", res1);

    
    cv::Mat res = table_roi.clone();
    for (size_t i = 0; i < circles.size(); i++) {
        cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int radius = cvRound(circles[i][2]);

        // Create a mask for the detected circle
        cv::Mat circleMask1 = cv::Mat::zeros(mask_col.size(), CV_8UC1);
        cv::circle(circleMask1, center, radius, cv::Scalar(255), -1);

        // Check the area inside the circle in both the segmentation mask and the thresholded mask
        cv::Mat segCircle1, threshCircle1;
        cv::bitwise_and(ROI, ROI, segCircle1, circleMask1);
        cv::bitwise_and(~mask_col, ~mask_col, threshCircle1, circleMask1);
        /* cv::imshow("circleMask", circleMask);
        cv::imshow("~mask_col", ~mask_col);
        cv::imshow("tresh circle", threshCircle); */
        
        // Calculate the area of the white pixels in the segmentation mask and black pixels in the thresholded mask
        double area1 = cv::countNonZero(circleMask1);
        double whiteSegArea1 = cv::countNonZero(segCircle1);
        double blackThreshArea1 = cv::countNonZero(threshCircle1);
        std::cout << "black  "<< blackThreshArea1 << "white  "<< whiteSegArea1;

        if (whiteSegArea1/area1 > 0.6 && blackThreshArea1/area1 > 0.5 && blackThreshArea1/whiteSegArea1 > 0.6) { 
            // Draw the circle
            cv::circle(res, center, radius, cv::Scalar(0, 255, 0), 1);
        }
    }
    cv::imshow("Detected Balls", res);

    cv::waitKey(0);

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


