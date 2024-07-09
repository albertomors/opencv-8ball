/*AUTHOR: Girardello Sofia */
#include "ballDetection.h"

// Constructor to initialize the frame
ballDetector::ballDetector(const cv::Mat& currentFrame) {
    this->frame = currentFrame.clone();  // Ensure a deep copy
}

void ballDetector::detectBalls() {

    //cv::Mat mask = cv::imread("Dataset/test.jpg");
    //bitwise_and(frame, ~mask, frame);
    cv::imshow("prova", frame);

    cv::Mat img_gray;
    cvtColor(frame, img_gray, cv::COLOR_RGB2GRAY);
    cv::Mat blurred_img;
    cv::GaussianBlur(img_gray, blurred_img, cv::Size(7,7), 0);
    cv::Mat edges;
    cv::Canny(blurred_img, edges, 0, 50);
    cv::imshow("canny", edges);

    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(edges, circles, cv::HOUGH_GRADIENT, 3, frame.rows/15, 450, 30, 7, 10);

    cv::Mat result = frame.clone();
    for(int i = 0; i < circles.size(); i++){
        cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int radius = cvRound(circles[i][2]);
        //circle(result, center, 3, cv::Scalar(0,255,0), -1, 8, 0 );
        circle(result, center, radius, cv::Scalar(0,0,255), 3, 8, 0 );
    }
    cv::imshow("result", result);


    for (size_t i = 0; i < circles.size(); i++) {
        cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int radius = cvRound(circles[i][2]);

        // Create a mask for the detected circle
        cv::Mat circleMask = cv::Mat::zeros(frame.size(), CV_8UC1);
        cv::circle(circleMask, center, radius, cv::Scalar(255), -1);

        // Extract the ROI from the original frame
        cv::Mat ballROI;
        frame.copyTo(ballROI, circleMask);

        // Analyze the ROI to determine if it is striped or solid
        bool isStriped = analyzeBallPattern(ballROI, center, radius);

        // Draw the detected circle and label it
        cv::Scalar color = isStriped ? cv::Scalar(255, 0, 0) : cv::Scalar(0, 255, 0);  // Blue for striped, Green for solid
        cv::circle(result, center, radius, color, 3, 8, 0);
    }
    cv::imshow("calssification result", result);


}

bool ballDetector::analyzeBallPattern(const cv::Mat& ballROI, const cv::Point& center, int radius) {
    // Convert the ROI to grayscale
    cv::Mat gray;
    cv::cvtColor(ballROI, gray, cv::COLOR_BGR2GRAY);
    //cv::equalizeHist(gray, gray);

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
    return whitePercentage > 10.0;
}