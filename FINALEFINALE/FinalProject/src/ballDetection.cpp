/*
    AUTHOR: Girardello Sofia
    DATE: 2024-07-21 
    FILE: ballDetector.cpp
    DESCRIPTION: Implements the ball detection algorithm using Hough Circles, colour masks and tracker center matching.

    CLASSES:
    - class ballDetector: Class for detecting and classifying balls.

    MAIN FUNCTIONS:
    - ballDetector(): Constructor to initialize the ballDetector object.
    - void detectBalls(...): Handles the detection and calls the other functions.
    - void applyColourDetection(...): Performs detection using Hough Transform on colour masks.
    - void selectBalls(...): Select just the acceptable balls using colour thresholding masks.
    - BallPattern analyzeBallPattern(...): Analyzes the ball pattern based on its appearance.
    - void classifyBalls(...): Classifies each ball given its colour and pattern analytics.
    - void detectBallsFinalFrame(...): Detects balls in the final frame and matches with tracker centers.
    - void saveInfo(...): Stores important information about each single selected ball.

    ADDITIONAL FUNCTIONS: 
    - enhanceContrast(...): Enhances the contrast of the input image using CLAHE (Contrast Limited Adaptive Histogram Equalization) in the LAB color space. Improves the visibility of features in the image.
    - detectedBallsData(...): Constructs a matrix with information about detected balls, including their bounding boxes and IDs.
    - createLabeledImage(...): Creates a labeled image that visualizes detected balls with their corresponding IDs.

    EXAMPLES:
    - Input: A frame from a video feed with balls visible.
    - Output: A frame with detected circles overlaid, showing both the detected circles and their classes.
*/

#include "ballDetection.h"

//------------ ADDITIONAL FUNCTIONS ------------

cv::Mat enhanceContrast(cv::Mat& frame) {

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
    cv::Mat out; //NEW
    cv::cvtColor(lab, out, cv::COLOR_Lab2BGR);
    return out; //NEW
}


cv::Mat averageColourThresholding(const cv::Mat& table_roi, const int areaSize){

    // Define the area to compute the average
    int startX = (table_roi.cols - areaSize) / 2;
    int startY = (table_roi.rows - areaSize) / 2;

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
    cv::Mat mask;
    cv::inRange(hsv_img, cv::Scalar(hsvColor[0] - 7.3, 70, 70), cv::Scalar(hsvColor[0] + 11.9, 255, 255), mask);

    return mask;

}


cv::Mat detectedBallsData(std::vector<cv::Rect>& bboxes, std::vector<int>& id_balls){

    // Create an empty matrix to save the data
    cv::Mat detectedBallsData = cv::Mat::zeros(static_cast<int>(bboxes.size()), 5, CV_16U);

    // Iterate over all the bounding boxes to save the data in the matrix
    for (size_t i = 0; i < bboxes.size(); ++i) {
        detectedBallsData.at<uint16_t>(i, 0) = static_cast<uint16_t>(bboxes[i].x);
        detectedBallsData.at<uint16_t>(i, 1) = static_cast<uint16_t>(bboxes[i].y);
        detectedBallsData.at<uint16_t>(i, 2) = static_cast<uint16_t>(bboxes[i].width);
        detectedBallsData.at<uint16_t>(i, 3) = static_cast<uint16_t>(bboxes[i].height);
        detectedBallsData.at<uint16_t>(i, 4) = static_cast<uint16_t>(id_balls[i]);
    }

    return detectedBallsData;

}


cv::Mat createLabeledImage(cv::Mat ROI, std::vector<cv::Point2f>& centers, std::vector<cv::Rect>& bboxes, std::vector<int>& id_balls){

    // Create an empty grayscale image
    cv::Mat labeledImage = cv::Mat::zeros(ROI.size(), CV_8UC1);

    // Label the table field with pixel value 5
    for (int y = 0; y < ROI.rows; ++y) {
        for (int x = 0; x < ROI.cols; ++x) {
            if (ROI.at<uchar>(y, x) > 0) {
                labeledImage.at<uchar>(y, x) = 5; 
            }
        }
    }

    // Draw the detected circles on the labeled image
    for (size_t i = 0; i < centers.size(); ++i) {
        cv::Point2f center = centers[i];
        int radius = bboxes[i].width / 2;
        int id = id_balls[i];

        /* --Debug: Print circle center and radius
        std::cout << "Ball ID: " << id << " Center: " << center << " Radius: " << radius << std::endl;
        */

        // Ensure the circle parameters are within bounds
        int x_start = std::max(0, static_cast<int>(center.x) - radius);
        int y_start = std::max(0, static_cast<int>(center.y) - radius);
        int x_end = std::min(labeledImage.cols, static_cast<int>(center.x) + radius);
        int y_end = std::min(labeledImage.rows, static_cast<int>(center.y) + radius);

        for (int y = y_start; y < y_end; ++y) {
            for (int x = x_start; x < x_end; ++x) {
                if (cv::norm(center - cv::Point2f(static_cast<float>(x), static_cast<float>(y))) <= radius) {
                    labeledImage.at<uchar>(y, x) = id; // Label the ball with its ID
                }
            }
        }
    }

    return labeledImage;

}


//------------ MAIN FUNCTIONS ------------

// Constructor 
ballDetector::ballDetector() {
}


void ballDetector::detectBalls(const cv::Mat& currentFrame, const cv::Mat& ROI, const std::vector<cv::Point2f> table_corners) {

    // Input validation
    if (currentFrame.size() != ROI.size() || currentFrame.type() != CV_8UC3 || ROI.type() != CV_8UC1) {
        std::cerr << "Error: Invalid input images!" << std::endl;
        return;
    }

    currentFrame.copyTo(this->table_roi, ROI); // Mask the current frame with ROI

    // Define the needed variables
    cv::Mat colour_mask;
    std::vector<cv::Vec3f> circles;

    // Apply a colour thresholding and Hough Transform using the dedicated function
    applyColourDetection(this->table_roi, colour_mask, circles);        

    // Clear previous centers and trajectories
    this->centers.clear();
    this->balls.clear();
    this->bboxes.clear();
    this->id_balls.clear();

    // Recall to the function that filters the balls found by HoughCircles
    std::vector<BallPattern> ballPatterns;
    ballPatterns = selectBalls(ROI, colour_mask, circles, table_corners);     // Save the selected balls

    // Recall to the function that classifies the selected balls
    classifyBalls(ballPatterns);


    // Create the matrices that characterize this frame (will be used for metrics purposes)
    this->bbox_data = detectedBallsData(this->bboxes, this->id_balls);
    this->classification_res = createLabeledImage(ROI, this->centers, this->bboxes, this->id_balls);

}

void ballDetector::applyColourDetection(cv::Mat& frame, cv::Mat& colour_mask, std::vector<cv::Vec3f>& circles) {

    // Enhance contrast
    cv::Mat edit = enhanceContrast(frame); //NEW

    // Define the size of the area around the center to compute the average color
    int areaSize = 50;

    // Perform colour thresholding to select just the table area (excluded balls)
    colour_mask = averageColourThresholding(edit, areaSize); //NEW

    // Find the balls using Hough Tranform 
    /*
    CONFIGURATION PARAMETERS:
    - Hough Circle detection parameters: 
        - dp: Inverse ratio of accumulator resolution to image resolution.
        - minDist: Minimum distance between circle centers.
        - param1: Higher threshold for Canny edge detector.
        - param2: Accumulator threshold for circle detection.
        - minRadius: Minimum circle radius.
        - maxRadius: Maximum circle radius.
    */
    //cv::HoughCircles(colour_mask, circles, cv::HOUGH_GRADIENT, 1.7, colour_mask.rows / 24, 30, 10.7, 5, 15);
    cv::HoughCircles(colour_mask, circles, cv::HOUGH_GRADIENT, 1.5, colour_mask.rows / 24, 30, 10.7, 5, 15);

    /* --Debug: Draw detected circles on the original table_roi image
    cv::Mat result_hough = table_roi.clone();
    for (size_t i = 0; i < circles.size(); i++) {
        cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int radius = cvRound(circles[i][2]);
        cv::circle(result_hough, center, radius, cv::Scalar(0, 255, 0), 2);
    }
    cv::imshow("Detected balls before filtering", result_hough);
    cv::waitKey(1);
    */

}

std::vector<BallPattern> ballDetector::selectBalls(const cv::Mat& ROI, const cv::Mat& mask, const std::vector<cv::Vec3f>& circles, const std::vector<cv::Point2f> table_corners) {

    std::vector<BallPattern> ballPatterns;

    double cornerDistanceThreshold = 60.0; // Min distance from table corner to be considered valid

    for (size_t i = 0; i < circles.size(); i++) {

        // Extract circle information
        cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int radius = cvRound(circles[i][2]);

        // Check if the detected circle is too close to any of the table corners
        bool tooCloseToCorner = false;
        cv::Point2f centerFloat(center.x, center.y); // Convert to float point for comparison
        for (const auto& corner : table_corners) {
            if (cv::norm(centerFloat - corner) < cornerDistanceThreshold) {
                tooCloseToCorner = true;
                break;
            }
        }

        // Skip this circle if it's too close to a corner
        if (tooCloseToCorner) {
            continue;
        }

        // Create a mask for the detected circle
        cv::Mat circleMask = cv::Mat::zeros(mask.size(), CV_8UC1);
        cv::circle(circleMask, center, radius, cv::Scalar(255), -1);

        // Check the area inside the circle in both the segmentation mask and the thresholded mask
        cv::Mat segCircle, threshCircle;
        cv::bitwise_and(ROI, ROI, segCircle, circleMask);
        cv::bitwise_and(~mask, ~mask, threshCircle, circleMask);
        
        // Calculate the area of the white pixels in the segmentation mask and black pixels in the thresholded mask
        double circleArea = cv::countNonZero(circleMask);
        double whiteSegArea = cv::countNonZero(segCircle);
        double blackThreshArea = cv::countNonZero(threshCircle);

        // Filter the balls using the colour mask and the ROI analysis
        if (whiteSegArea/circleArea > 0.7 && blackThreshArea/circleArea > 0.6 && blackThreshArea/whiteSegArea > 0.4) { 

            // Recall to the function that analizes the pattern/colour of the ball
            BallPattern pattern = analyzeBallPattern(this->table_roi, circleMask);
            ballPatterns.push_back(pattern);  

            // Recall to the function that saves the important info of the current ball
            saveInfo(center, radius);
        }
    }            

    return ballPatterns;

}


BallPattern ballDetector::analyzeBallPattern(const cv::Mat& ballROI, const cv::Mat& circleMask) {

    // Desaturate the image by converting into grayscale
    cv::Mat gray;
    cv::cvtColor(ballROI, gray, cv::COLOR_BGR2GRAY);

    // Select the white areas and the black areas of the image and create two masks
    cv::Mat binary_white;
    cv::threshold(gray, binary_white, 190, 255, cv::THRESH_BINARY);
    cv::Mat binary_black;
    cv::threshold(gray, binary_black, 50, 255, cv::THRESH_BINARY);
    cv::bitwise_not(binary_black, binary_black);

    // Select just the area related to the current studied ball
    cv::Mat maskedBinary_white;
    binary_white.copyTo(maskedBinary_white, circleMask);
    cv::Mat maskedBinary_black;
    binary_black.copyTo(maskedBinary_black, circleMask);

    // Analize the percentage of black and white colour in that area
    int whitePixels = cv::countNonZero(maskedBinary_white);
    int blackPixels = cv::countNonZero(maskedBinary_black);
    int totalPixels = cv::countNonZero(circleMask);
    double whitePercentagePattern = (double)whitePixels / totalPixels * 100;
    double blackPercentagePattern = (double)blackPixels / totalPixels * 100;

    return {whitePercentagePattern, blackPercentagePattern, 0};
}


void ballDetector::classifyBalls(std::vector<BallPattern>& ballPatterns){

    // Find the balls with the maximum ratio of black and white colour
    double maxWhitePercentage = 0;
    double maxBlackPercentage = 0;
    int whiteBallIndex = -1;
    int blackBallIndex = -1;

    for (size_t i = 0; i < ballPatterns.size(); ++i) {
        if (ballPatterns[i].whitePercentage > maxWhitePercentage) {
            maxWhitePercentage = ballPatterns[i].whitePercentage;
            whiteBallIndex = i;
        }
        if (ballPatterns[i].blackPercentage > maxBlackPercentage) {
            maxBlackPercentage = ballPatterns[i].blackPercentage;
            blackBallIndex = i;
        }
    }

    // --Debug cv::Mat res = table_roi.clone();
    for (size_t i = 0; i < ballPatterns.size(); ++i) {
        if (i == whiteBallIndex) {
             ballPatterns[i].id = 1; // White ball
        } else if (i == blackBallIndex) {
            ballPatterns[i].id = 2; // Black ball
        } else if (ballPatterns[i].whitePercentage > 13) {
            ballPatterns[i].id = 4; // Striped ball
        } else {
             ballPatterns[i].id = 3; // Solid ball
        }

        /* --Debug: visualize the balls and the relative classes
        cv::Scalar color;
        switch (ballPatterns[i].id) {
            case 1: color = cv::Scalar(255, 255, 255); break;
            case 2: color = cv::Scalar(0, 0, 0); break;
            case 3: color = cv::Scalar(0, 0, 255); break;
            case 4: color = cv::Scalar(255, 0, 0); break;
            default: color = cv::Scalar(0, 255, 0); break;
        }
        int radius = this->bboxes[i].width / 2;
        cv::Point2f center = this->centers[i];
        cv::circle(res, center, radius, color, 3, 8, 0);
        */

        this->id_balls.push_back(ballPatterns[i].id);

    }
    /* --Debug 
    cv::imshow("Detected Balls", res);
    cv::waitKey(0);*/
}


void ballDetector::detectBallsFinalFrame(const cv::Mat& frame, const cv::Mat& ROI, const std::vector<cv::Point2f>& trackerCenters, const std::vector<int>& trackerIDs, const std::vector<cv::Point2f>& table_corners) {

    // Ensure the size of trackerCenters and trackerIDs are the same
    assert(trackerCenters.size() == trackerIDs.size());

    // Parameters for circle detection
    double maxDistance = 20.0; // Max distance to consider a circle as near a tracker
    int defaultRadius = 10;    // Default radius if no circle is found

    cv::Mat final_table_roi;
    frame.copyTo(final_table_roi, ROI); // Mask the current frame with ROI

    // Define the needed variables
    cv::Mat colour_mask;
    std::vector<cv::Vec3f> circles;

    // Apply a colour thresholding and Hough Transform using the dedicated function
    applyColourDetection(this->table_roi, colour_mask, circles); 

    // Convert detected circles into DetectedCircle objects
    struct DetectedCircle {
        cv::Point2f center;
        int radius;
    };
    std::vector<DetectedCircle> detectedCircles;
    for (const auto& circle : circles) {
        DetectedCircle detectedCircle;
        detectedCircle.center = cv::Point2f(circle[0], circle[1]);
        detectedCircle.radius = static_cast<int>(circle[2]);
        detectedCircles.push_back(detectedCircle);
    }
    
    // Iterate over each tracker
    for (size_t i = 0; i < trackerCenters.size(); ++i) {
        cv::Point2f trackerCenter = trackerCenters[i];
        int trackerID = trackerIDs[i];

        bool circleFound = false;
        cv::Point2f circleCenter = trackerCenter;
        int circleRadius = defaultRadius;

        // Check if any detected circle is near this tracker
        for (const auto& detectedCircle : detectedCircles) {
            if (cv::norm(trackerCenter - detectedCircle.center) < maxDistance) {
                circleCenter = detectedCircle.center;
                circleRadius = detectedCircle.radius;
                circleFound = true;
                break; // No need to check other circles once a match is found
            }
        }

        // Save information
        saveInfo(circleCenter, circleRadius);
        this->id_balls.push_back(trackerID);

    }

    // Create the matrices that characterize this frame (will be used for metrics purposes)
    this->bbox_data = detectedBallsData(this->bboxes, this->id_balls);
    this->classification_res = createLabeledImage(ROI, this->centers, this->bboxes, this->id_balls);

}


void ballDetector::saveInfo(const cv::Point center, const int radius){

    // Save the center
    this->centers.push_back(cv::Point2f(center.x, center.y));

    // Create and save the rectangle around the center for tracking purposes
    int x = center.x - radius;
    int y = center.y - radius;

    int rect_size = 3 * radius;
    cv::Rect ballRect(x, y, rect_size, rect_size);
    this->balls.push_back(ballRect);

    // Create bounding box from center and radius and save it
    int width = 2 * radius;
    int height = 2 * radius;

    cv::Rect boundingBox(x, y, width, height);
    this->bboxes.push_back(boundingBox);

}
