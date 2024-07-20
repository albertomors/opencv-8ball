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
    cv::waitKey(1);

    //cv::GaussianBlur(table_roi, table_roi, cv::Size(9, 9), 2);
    // Enhance contrast
    cv::Mat table_enhanced;
    currentFrame.copyTo(table_enhanced, ROI); // Mask the current frame with ROI
    enhanceContrast(table_enhanced);

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
    cv::Mat centerArea = table_enhanced(centerRect);

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
    cv::inRange(hsv_img, cv::Scalar(hsvColor[0] - 7.5, 80, 80), cv::Scalar(hsvColor[0] + 11.9, 255, 255), mask_col);
    cv::imshow("Colour Thresholded Mask", mask_col);

    cv::Mat gray;
    cv::cvtColor(table_roi, gray, cv::COLOR_BGR2GRAY);

    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(mask_col, circles, cv::HOUGH_GRADIENT, 1.25, mask_col.rows / 26, 30, 8, 5, 15);

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
        //std::cout << "black  "<< blackThreshArea1 << "white  "<< whiteSegArea1;

        if (whiteSegArea1/area1 > 0.8 && blackThreshArea1/area1 > 0.5 && blackThreshArea1/whiteSegArea1 > 0.4) { 

            // Create a mask for the detected circle
            cv::Mat circleMask = cv::Mat::zeros(currentFrame.size(), CV_8UC1);
            cv::circle(circleMask, center, radius, cv::Scalar(255), -1);

            // Extract the ROI from the original frame
            cv::Mat ballROI;
            currentFrame.copyTo(ballROI, circleMask);

            // Analyze the ROI to determine if it is striped or solid
            int ballType = analyzeBallPattern(ballROI, center, radius);

            // Set the color and ID based on the ball type
            cv::Scalar color;
            int ballID;
            switch (ballType) {
                case 1: // White
                    color = cv::Scalar(255, 255, 255); // White
                    ballID = 1;
                    break;
                case 2: // Black
                    color = cv::Scalar(0, 0, 0);    // Black
                    ballID = 2;
                    break;
                case 3: // Solid
                    color = cv::Scalar(0, 0, 255);  // Red
                    ballID = 3;
                    break;
                case 4: // Striped
                    color = cv::Scalar(255, 0, 0);  // Blue
                    ballID = 4;
                    break;
                default:
                    color = cv::Scalar(0, 255, 0);  // Default to green
                    ballID = 5;
                    break;
            }

            // Draw the detected circle and label it
            cv::circle(res, center, radius, color, 3, 8, 0);
            this->centers.push_back(cv::Point2f(center.x, center.y));
            this->id_balls.push_back(ballID);

            // Create and save the rectangle around the center

            int x = center.x - radius;
            int y = center.y - radius;

            int rect_size = 3 * radius;
            cv::Rect ballRect(x, y, rect_size, rect_size);
            this->balls.push_back(ballRect);


            // Create bounding box from center and radius
            int width = 2 * radius;
            int height = 2 * radius;

            cv::Rect boundingBox(x, y, width, height);
            this->bboxes.push_back(boundingBox);


        }
    }

    cv::Mat detectedBallsData = cv::Mat::zeros(static_cast<int>(balls.size()), 5, CV_16U);

    for (size_t i = 0; i < balls.size(); ++i) {
        detectedBallsData.at<uint16_t>(i, 0) = static_cast<uint16_t>(bboxes[i].x);
        detectedBallsData.at<uint16_t>(i, 1) = static_cast<uint16_t>(bboxes[i].y);
        detectedBallsData.at<uint16_t>(i, 2) = static_cast<uint16_t>(bboxes[i].width);
        detectedBallsData.at<uint16_t>(i, 3) = static_cast<uint16_t>(bboxes[i].height);
        detectedBallsData.at<uint16_t>(i, 4) = static_cast<uint16_t>(id_balls[i]);
    }


    this->bbox_data = detectedBallsData;

    cv::Mat labeledImage = cv::Mat::zeros(currentFrame.size(), CV_8UC1);

    for (int y = 0; y < ROI.rows; ++y) {
        for (int x = 0; x < ROI.cols; ++x) {
            if (ROI.at<uchar>(y, x) > 0) {
                labeledImage.at<uchar>(y, x) = 5;
            }
        }
    }

    for (size_t i = 0; i < this->balls.size(); ++i) {
        cv::Rect bbox = this->balls[i];
        int id = this->id_balls[i];

        for (int y = bbox.y; y < bbox.y + bbox.height; ++y) {
            for (int x = bbox.x; x < bbox.x + bbox.width; ++x) {
                if (cv::norm(this->centers[i] - cv::Point2f(x, y)) <= bbox.width / 2.0) {
                    labeledImage.at<uchar>(y, x) = id;
                }
            }
        }
    }

    this->classification_res = labeledImage;

    

    cv::imshow("Detected Balls", res);

    // Continue with the existing pattern analysis for striped and solid balls
    cv::Mat gray1;
    cv::cvtColor(currentFrame, gray1, cv::COLOR_BGR2GRAY);

    cv::Mat binary_white;
    cv::threshold(gray1, binary_white, 175, 255, cv::THRESH_BINARY);
    //cv::imshow("binary white", binary_white);
    cv::Mat binary_black;
    cv::threshold(gray1, binary_black, 50, 255, cv::THRESH_BINARY);
    //cv::imshow("binary black", binary_black);

    cv::waitKey(0);

}


int ballDetector::analyzeBallPattern(const cv::Mat& ballROI, const cv::Point& center, int radius) {
    
    cv::Mat gray;
    cv::cvtColor(ballROI, gray, cv::COLOR_BGR2GRAY);

    cv::Mat binary_white;
    cv::threshold(gray, binary_white, 170, 255, cv::THRESH_BINARY);
    cv::Mat binary_black;
    cv::threshold(gray, binary_black, 50, 255, cv::THRESH_BINARY);
    cv::bitwise_not(binary_black, binary_black);

    cv::Mat circleMask = cv::Mat::zeros(binary_white.size(), CV_8UC1);
    cv::circle(circleMask, center, radius, cv::Scalar(255), -1);

    cv::Mat maskedBinary_white;
    binary_white.copyTo(maskedBinary_white, circleMask);
    cv::Mat maskedBinary_black;
    binary_black.copyTo(maskedBinary_black, circleMask);
    //cv::imshow("maskedBinary_black", maskedBinary_black);
    //cv::imshow("maskedBinary_white", maskedBinary_white);

    int whitePixels = cv::countNonZero(maskedBinary_white);
    int blackPixels = cv::countNonZero(maskedBinary_black);
    int totalPixels = cv::countNonZero(circleMask);
    double whitePercentagePattern = (double)whitePixels / totalPixels * 100;
    double blackPercentagePattern = (double)blackPixels / totalPixels * 100;

    //std::cout << "white percentage  "<< whitePercentagePattern << "black  "<< blackPercentagePattern;

    // Determine if the ball is 
    if(whitePercentagePattern > 45) return 1;
    else if (blackPercentagePattern > 50) return 2;
    else if (whitePercentagePattern > 13) return 4;
    else return 3;

}


