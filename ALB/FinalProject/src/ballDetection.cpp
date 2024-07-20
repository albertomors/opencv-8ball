/*
AUTHOR: Girardello Sofia 
*/

#include "ballDetection.h"

void enhanceContrast(cv::Mat& frame) {
    // Convert the frame to the LAB color space
    cv::Mat lab;
    cv::cvtColor(frame, lab, cv::COLOR_BGR2Lab);

    // Split the LAB image into separate channels
    std::vector<cv::Mat> lab_channels(3);
    cv::split(lab, lab_channels);

    // Apply CLAHE to the L-channel
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
    clahe->setClipLimit(7.0);
    cv::Mat l_channel;
    clahe->apply(lab_channels[0], l_channel);

    // Merge the CLAHE enhanced L-channel back with the original A and B channels
    l_channel.copyTo(lab_channels[0]);
    cv::merge(lab_channels, lab);

    // Convert the LAB image back to BGR color space
    cv::cvtColor(lab, frame, cv::COLOR_Lab2BGR);
}

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
    //cv::waitKey(1);

    //cv::GaussianBlur(table_roi, table_roi, cv::Size(9, 9), 2);
    // Enhance contrast
    enhanceContrast(table_roi);

    // Convert the frame to HSV
    /*cv::Mat hsv_img;
    cv::cvtColor(table_roi, hsv_img, cv::COLOR_BGR2HSV);

    // Thresholding based on hue to isolate balls
    cv::Mat mask_col;
    cv::inRange(hsv_img, cv::Scalar(color - 2.5, 0, 0), cv::Scalar(color + 3, 255, 255), mask_col);
    cv::imshow("Thresholded Mask", mask_col);*/

    // Define the size of the area around the center to compute the average color
    int areaSize = 50; // Adjust this size as needed
    int startX = (currentFrame.cols - areaSize) / 2;
    int startY = (currentFrame.rows - areaSize) / 2;

    cv::Rect centerRect(startX, startY, areaSize, areaSize);
    cv::Mat centerArea = table_roi(centerRect);

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
    cv::inRange(hsv_img, cv::Scalar(hsvColor[0] - 10, 80, 80), cv::Scalar(hsvColor[0] + 13.9, 255, 255), mask_col);
    //cv::imshow("Colour Thresholded Mask", mask_col);

    cv::Mat gray;
    cv::cvtColor(table_roi, gray, cv::COLOR_BGR2GRAY);

    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(mask_col, circles, cv::HOUGH_GRADIENT, 1.03, mask_col.rows / 26, 30, 7.45, 5, 15);

    // Draw detected circles on the original table_roi image
    cv::Mat res1 = table_roi.clone();
    for (size_t i = 0; i < circles.size(); i++) {
        cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int radius = cvRound(circles[i][2]);

        // Draw the circle
        cv::circle(res1, center, radius, cv::Scalar(0, 255, 0), 2);
    }
    //cv::imshow("Detected Balls before", res1);

    
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
        //std::cout << "black  "<< blackThreshArea1 << "white  "<< whiteSegArea1;

        if (whiteSegArea1/area1 > 0.8 && blackThreshArea1/area1 > 0.6 && blackThreshArea1/whiteSegArea1 > 0.4) { 

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
            cv::circle(res, center, radius, color, 1, 8, 0);
            this->centers.push_back(cv::Point2f(center.x, center.y));
            int id_solid = 1;
            int id_striped = 2;
            if (isStriped) this->id_balls.push_back(id_striped);
            else this->id_balls.push_back(id_solid);

            // Create and save the rectangle around the center
            int rect_size = radius*4;
            cv::Rect ballRect(center.x - radius, center.y - radius, rect_size, rect_size);
            this->balls.push_back(ballRect);
        }
    }
    cv::imshow("Detected Balls", res);
    //cv::waitKey(1);
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
    return whitePercentage > 5.8;
}


