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

void ballDetector::detectBalls(const cv::Mat& currentFrame, const cv::Mat& ROI, const std::vector<cv::Point2f> table_corners) {

    // Input validation
    if (currentFrame.size() != ROI.size() || currentFrame.type() != CV_8UC3 || ROI.type() != CV_8UC1) {
        std::cerr << "Error: Invalid input images!" << std::endl;
        return;
    }

    this->roi = ROI;

    //cv::imshow("ROI", ROI);

    currentFrame.copyTo(this->table_roi, ROI); // Mask the current frame with ROI

    /* --Debug
    cv::imshow("ROI Mask Applied", table_roi);
    cv::waitKey(1);
    */

    // Enhance contrast
    enhanceContrast(table_roi);

    // Define the size of the area around the center to compute the average color
    int areaSize = 50;
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

    /* --Debug
    cv::imshow("Colour Thresholded Mask", mask_col);
    cv::waitKey(1);
    */

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

    /* --Debug
    cv::imshow("Detected Balls before", res1);
    cv::waitKey(1);
    */

    cv::Mat res = table_roi.clone();

    // Clear previous centers and trajectories
        this->centers.clear();
        this->balls.clear();
        this->id_balls.clear();

    std::vector<BallPattern> ballPatterns;

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
        
        /* --Debug
        cv::imshow("circleMask", circleMask);
        cv::imshow("~mask_col", ~mask_col);
        cv::imshow("tresh circle", threshCircle); 
        */
        
        // Calculate the area of the white pixels in the segmentation mask and black pixels in the thresholded mask
        double area1 = cv::countNonZero(circleMask1);
        double whiteSegArea1 = cv::countNonZero(segCircle1);
        double blackThreshArea1 = cv::countNonZero(threshCircle1);

        if (whiteSegArea1/area1 > 0.8 && blackThreshArea1/area1 > 0.6 && blackThreshArea1/whiteSegArea1 > 0.4) { 

            // Create a mask for the detected circle
            cv::Mat circleMask = cv::Mat::zeros(currentFrame.size(), CV_8UC1);
            cv::circle(circleMask, center, radius, cv::Scalar(255), -1);

            // Extract the ROI from the original frame
            cv::Mat ballROI;
            currentFrame.copyTo(ballROI, circleMask);

            BallPattern pattern = analyzeBallPattern(ballROI, center, radius);
            ballPatterns.push_back(pattern);              

    

            this->centers.push_back(cv::Point2f(center.x, center.y));

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

    // Classify balls based on their patterns
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

                cv::Scalar color;
                switch (ballPatterns[i].id) {
                    case 1: color = cv::Scalar(255, 255, 255); break;
                    case 2: color = cv::Scalar(0, 0, 0); break;
                    case 3: color = cv::Scalar(0, 0, 255); break;
                    case 4: color = cv::Scalar(255, 0, 0); break;
                    default: color = cv::Scalar(0, 255, 0); break;
                }

                cv::circle(res, cv::Point(cvRound(circles[i][0]), cvRound(circles[i][1])), cvRound(circles[i][2]), color, 3, 8, 0);
                this->id_balls.push_back(ballPatterns[i].id);
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

    // Label the table field
    for (int y = 0; y < ROI.rows; ++y) {
        for (int x = 0; x < ROI.cols; ++x) {
            if (ROI.at<uchar>(y, x) > 0) {
                labeledImage.at<uchar>(y, x) = 5; // Label the table field with pixel value 5
            }
        }
    }

    // Draw the detected circles on the labeled image
    for (size_t i = 0; i < this->centers.size(); ++i) {
        cv::Point2f center = this->centers[i];
        int radius = this->bboxes[i].width / 2;
        int id = this->id_balls[i];

        // --Debug: Print circle center and radius
        //std::cout << "Ball ID: " << id << " Center: " << center << " Radius: " << radius << std::endl;

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

    this->classification_res = labeledImage;

    cv::imshow("Detected Balls", res);
    cv::waitKey(1);
}


BallPattern ballDetector::analyzeBallPattern(const cv::Mat& ballROI, const cv::Point& center, int radius) {

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

    /* --Debug
    cv::imshow("maskedBinary_black", maskedBinary_black);
    cv::imshow("maskedBinary_white", maskedBinary_white);
    */

    int whitePixels = cv::countNonZero(maskedBinary_white);
    int blackPixels = cv::countNonZero(maskedBinary_black);
    int totalPixels = cv::countNonZero(circleMask);
    double whitePercentagePattern = (double)whitePixels / totalPixels * 100;
    double blackPercentagePattern = (double)blackPixels / totalPixels * 100;

    return {whitePercentagePattern, blackPercentagePattern, 0};
}

void ballDetector::detectBallsFinalFrame(const cv::Mat& frame, const cv::Mat& ROI, const std::vector<cv::Point2f>& trackerCenters, const std::vector<int>& trackerIDs, const std::vector<cv::Point2f>& table_corners) {

    cv::Mat new_img;
    cv::imshow("frame", frame);
    cv::imshow("roi", roi);
    frame.copyTo(new_img, ROI);
    cv::imshow("risultato Debug", new_img);

    // Convert the ROI to grayscale for Hough Circle detection
    cv::Mat gray;
    try {
        cv::cvtColor(new_img, gray, cv::COLOR_BGR2GRAY);
    } catch (const cv::Exception& e) {
        std::cerr << "Error converting image to grayscale: " << e.what() << std::endl;
        return;
    }

    // Vector to store detected circles
    std::vector<cv::Vec3f> circles;

    // Perform Hough Circle detection
    cv::HoughCircles(gray, circles, cv::HOUGH_GRADIENT, 2.7, 30, 85, 30, 4, 15);

    // Create a result image to visualize the detected circles
    cv::Mat res = frame.clone(); // Clone the original frame to overlay circles

    // Vector to store detected circles that match tracker centers
    std::vector<std::tuple<cv::Point2f, int, int>> matchedCircles;

    // Draw all detected circles on the result image
    for (const auto& circle : circles) {
        cv::Point2f circleCenter(circle[0], circle[1]);
        int circleRadius = static_cast<int>(circle[2]);

        // Check if this circle is close to any tracker center
        for (size_t i = 0; i < trackerCenters.size(); ++i) {
            double distance = cv::norm(circleCenter - trackerCenters[i]);
            if (distance <= circleRadius*2) {
                // Save the circle details and associated ball ID
                matchedCircles.emplace_back(circleCenter, circleRadius, trackerIDs[i]);

                // Draw the circle center
                cv::circle(res, circleCenter, 3, cv::Scalar(255, 0, 0), -1); // Red for center

                // Draw the circle outline
                cv::circle(res, circleCenter, circleRadius, cv::Scalar(0, 255, 0), 2); // Green for circle outline
                break; // Stop checking once we find a match
            }
        }
    }

    // Display the result with all detected circles
    cv::namedWindow("Detected Circles Debug", cv::WINDOW_NORMAL);
    cv::imshow("Detected Circles Debug", res);
    cv::waitKey(0); 
}


