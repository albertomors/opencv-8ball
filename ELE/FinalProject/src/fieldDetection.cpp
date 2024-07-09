/*AUTHOR: Fresco Eleonora */

#include "fieldDetection.h"

// Constructor to initialize the frame
fieldDetector::fieldDetector(const cv::Mat& currentFrame) {
    this->frame = currentFrame.clone();  // Ensure a deep copy
}

// Simplified version
/*cv::Vec2f findIntersection(cv::Vec2f line1, cv::Vec2f line2) {
    float t0 = line1[1];
    float t1 = line2[1];
    float r0 = line1[0];
    float r1 = line2[0];
    float det = std::cos(t0) * std::sin(t1) - std::sin(t0) * std::cos(t1);

    if (det == 0) {
        return cv::Vec2f(-1, -1); // Lines are parallel
    }

    float x = (r0 * std::sin(t1) - r1 * std::sin(t0)) / det;
    float y = (r1 * std::cos(t0) - r0 * std::cos(t1)) / det;

    return cv::Vec2f(x, y);
}*/

/*std::vector<cv::Point> findROI(const std::vector<cv::Vec2f>& intersections) {
    std::vector<cv::Point> roi;
    if (intersections.size() < 4) {
        return roi; // Not enough points to form a polygon
    }

    // Convert points to cv::Point for convex hull calculation
    std::vector<cv::Point> points;
    for (const auto& pt : intersections) {
        points.push_back(cv::Point(cvRound(pt.x), cvRound(pt.y)));
    }

    // Find the convex hull of the intersections
    cv::convexHull(points, roi);
    return roi;
}*/

std::vector<cv::Point> findMaxContour(const cv::Mat& mask) {
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    double maxArea = 0;
    std::vector<cv::Point> maxContour;

    for (const auto& contour : contours) {
        double area = cv::contourArea(contour);
        if (area > maxArea) {
            maxArea = area;
            maxContour = contour;
        }
    }

    return maxContour;
}


void fieldDetector::detectField() {

    cv::Mat image = this->frame.clone();
    cv::Mat result = this->frame.clone();

    // Define the central region
    int centerX = image.cols / 2;
    int centerY = image.rows / 2;
    int regionSize = std::min(image.cols, image.rows) / 4; // Define size of central region
    cv::Rect centralRegion(centerX - regionSize / 2, centerY - regionSize / 2, regionSize, regionSize);

    // Extract the central region
    cv::Mat centralPart = image(centralRegion);

    // Convert to HSV color space
    cv::Mat hsvImage;
    cv::cvtColor(centralPart, hsvImage, cv::COLOR_BGR2HSV);

    // Compute the histogram
    int hBins = 50, sBins = 60;
    int histSize[] = { hBins, sBins };
    float hRanges[] = { 0, 180 };
    float sRanges[] = { 0, 256 };
    const float* ranges[] = { hRanges, sRanges };
    cv::MatND hist;
    int channels[] = { 0, 1 };

    cv::calcHist(&hsvImage, 1, channels, cv::Mat(), hist, 2, histSize, ranges, true, false);
    cv::normalize(hist, hist, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());

    // Find the peak in the histogram
    double maxVal = 0;
    int maxIdx[2] = { 0, 0 };
    cv::minMaxIdx(hist, 0, &maxVal, 0, maxIdx);

    cv::Scalar avgBrightness = cv::mean(hsvImage);
    // Extract the V channel and compute the median brightness
    std::vector<uchar> vValues;
    for (int i = 0; i < hsvImage.rows; i++) {
        for (int j = 0; j < hsvImage.cols; j++) {
            vValues.push_back(hsvImage.at<cv::Vec3b>(i, j)[2]);
        }
    }
    std::nth_element(vValues.begin(), vValues.begin() + vValues.size() / 2, vValues.end());
    float medianV = vValues[vValues.size() / 2];

    // Convert the peak histogram bin back to HSV value
    float hStep = 180.0 / hBins;
    float sStep = 256.0 / sBins;
    float h = maxIdx[0] * hStep;
    float s = maxIdx[1] * sStep;
    float v = medianV;

    // Convert HSV to BGR
    cv::Mat hsvColor(1, 1, CV_8UC3, cv::Scalar(h, s, v));
    cv::Mat bgrColor;
    cv::cvtColor(hsvColor, bgrColor, cv::COLOR_HSV2BGR);

    cv::Vec3b predominantColor = bgrColor.at<cv::Vec3b>(0, 0);
    cv::Mat colorImage(image.rows, image.cols, CV_8UC3, cv::Scalar(predominantColor[0], predominantColor[1], predominantColor[2]));
    cv::imshow("Predominant Color Image", colorImage);
  
    // Define range for BGR color filtering
    int threshold = 100; // Adjust this threshold as needed 80
    cv::Scalar lowerBound(
        std::max(predominantColor[0] - threshold, 0),
        std::max(predominantColor[1] - threshold, 0),
        std::max(predominantColor[2] - threshold, 0)
    );
    cv::Scalar upperBound(
        std::min(predominantColor[0] + threshold, 255),
        std::min(predominantColor[1] + threshold, 255),
        std::min(predominantColor[2] + threshold, 255)
    );

    // Create mask based on the BGR range
    cv::Mat mask_colour;
    cv::inRange(image, lowerBound, upperBound, mask_colour);
    cv::imshow("Mask based on colour", mask_colour);

    // Detect contours to find potential table boundaries
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask_colour, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    
    // Find the contour with the largest area (assuming it's the table)
    /*double maxArea = 0;
    std::vector<cv::Point> maxContour;
    for (const auto& contour : contours) {
        double area = cv::contourArea(contour);
        if (area > maxArea) {
            maxArea = area;
            maxContour = contour;
        }
    }*/
    // Use method
    std::vector<cv::Point> maxContour = findMaxContour(mask_colour);

    // Approximate the contour to a polygon (rectangle in this case)
    std::vector<cv::Point> approx;
    cv::approxPolyDP(maxContour, approx, cv::arcLength(maxContour, true) * 0.05, true); 

    // Draw maxContour
    cv::Mat contourMask = cv::Mat::zeros(this->frame.size(), CV_8UC1);
    //cv::drawContours(contourMask, contours, -1, cv::Scalar(255), cv::FILLED); // All contours are drawn
    cv::drawContours(contourMask, std::vector<std::vector<cv::Point>>{maxContour}, -1, cv::Scalar(255), cv::FILLED); // Only biggest contour is drawn
    cv::imshow("Contours Mask", contourMask);

    // Find connected components
    cv::Mat labels;
    int numComponents = cv::connectedComponents(contourMask, labels);
    //std::cout << "Number of connected components: " << numComponents << std::endl;

    // Normalize the label values for visualization
    cv::Mat normalizedLabels;
    labels.convertTo(normalizedLabels, CV_8U, 255.0 / (numComponents - 1));
    // Apply a colormap to better visualize the different components
    cv::Mat colorLabels;
    cv::applyColorMap(normalizedLabels, colorLabels, cv::COLORMAP_JET);
    // Display the connected components with color mapping
    //cv::imshow("Connected Components", colorLabels);

    // Count the number of pixels in each component
    std::vector<int> componentSizes(numComponents, 0);
    for (int i = 0; i < labels.rows; i++) {
        for (int j = 0; j < labels.cols; j++) {
            int label = labels.at<int>(i, j);
            if (label > 0) { // Ignore the background
                componentSizes[label]++;
            }
        }
    }

    // Find the component with the highest number of pixels
    int largestComponentLabel = 1;
    int maxSize = componentSizes[1];
    for (int i = 2; i < numComponents; i++) {
        if (componentSizes[i] > maxSize) {
            largestComponentLabel = i;
            maxSize = componentSizes[i];
        }
    }

    // Create a mask for the largest component
    cv::Mat largestComponentMask;
    largestComponentMask = cv::Mat::zeros(labels.size(), CV_8U);
    for (int i = 0; i < labels.rows; i++) {
        for (int j = 0; j < labels.cols; j++) {
            if (labels.at<int>(i, j) == largestComponentLabel) {
                largestComponentMask.at<uchar>(i, j) = 255;
            }
        }
    }

    cv::imshow("Largest Component", largestComponentMask);


    // Hough Lines Probabilistic
    cv::Mat HoughMask = cv::Mat::zeros(largestComponentMask.size(), CV_8UC1);
    std::vector<cv::Vec4i> linesP;
    cv::HoughLinesP(largestComponentMask, linesP, 1, CV_PI / 180, 1, 20, 40); //rho, theta, votes 3, minlinelength 50, maxlinegap 50
    
    for (size_t i = 0; i < linesP.size(); i++) {
        cv::Vec4i l = linesP[i];
        cv::line(result, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(255, 0, 0), 3, cv::LINE_AA);
        cv::line(HoughMask, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(255), 3, cv::LINE_AA);
    }

    // Keep only strongest lines
    /*double minLineLength = 10.0; //50
    std::vector<cv::Vec4i> filteredLinesP;
    for (size_t i = 0; i < linesP.size(); i++) {
        cv::Vec4i l = linesP[i];
        double lineLength = cv::norm(cv::Point(l[0], l[1]) - cv::Point(l[2], l[3]));
        if (lineLength >= minLineLength) {
            filteredLinesP.push_back(l);
        }
    }

    // Draw filtered lines on the result image--DEBUG
    for (size_t i = 0; i < filteredLinesP.size(); i++) {
        cv::Vec4i l = filteredLinesP[i];
        cv::line(result, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(255, 0, 0), 3, cv::LINE_AA);
        //cv::line(largestComponentMask, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(255, 0, 0), 3, cv::LINE_AA);
    }*/

    cv::imshow("Mask HoughP", HoughMask); //DEBUG

    //std::vector<std::vector<cv::Point>> contourMaskRefined;
    //cv::findContours(HoughMask, contourMaskRefined, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); //anzichè su largestcomponent

    //approach 1 
    /*std::vector<std::vector<cv::Point>> approx_contours(contourMaskRefined.size());
    for (size_t i = 0; i < contourMaskRefined.size(); ++i) {
        cv::approxPolyDP(contourMaskRefined[i], approx_contours[i], cv::arcLength(contourMaskRefined[i], true) * 0.05, true);
    }

    for (size_t i = 0; i < approx_contours.size(); ++i) {
        cv::Scalar colorYellow(0, 255, 255); // Yellow color (BGR)
        cv::drawContours(result, approx_contours, static_cast<int>(i), colorYellow, 2); // Draw contours in yellow
        cv::drawContours(this->frame, approx_contours, static_cast<int>(i), colorYellow, 2);
    }*/

    //approach 2
    /*std::vector<std::vector<cv::Point>> approx_contours(contourMaskRefined.size());
    std::vector<std::vector<cv::Point>> hulls(contourMaskRefined.size());

    for (size_t i = 0; i < contourMaskRefined.size(); ++i) {
        // Approximate each contour
        cv::approxPolyDP(contourMaskRefined[i], approx_contours[i], cv::arcLength(contourMaskRefined[i], true) * 0.05, true);
        // Apply convex hull to the approximated contour
        cv::convexHull(approx_contours[i], hulls[i]);
    }

    
    for (size_t i = 0; i < hulls.size(); ++i) {
        cv::Scalar colorYellow(0, 255, 255); // Yellow color (BGR)
        cv::drawContours(result, hulls, static_cast<int>(i), colorYellow, 2); // Draw convex hulls in yellow
        cv::drawContours(this->frame, hulls, static_cast<int>(i), colorYellow, 2);
    }
   cv::imshow("approach 2", result);*/

    //TILL HERE IS SOMETHING

    /*double maxAreaFinal = 0;
    std::vector<cv::Point> maxContourFinal;
    
    // Find the contour with the maximum area
    for (const auto& contour : contourMaskRefined) {
        double area = cv::contourArea(contour);
        if (area > maxAreaFinal) {
            maxAreaFinal = area;
            maxContourFinal = contour;
        }
    }*/
    // Use method
    std::vector<cv::Point> maxContourFinal = findMaxContour(HoughMask);

    // Approximate the largest contour to a polygon
    std::vector<cv::Point> approxFinal;
    double epsilon = 0.02 * cv::arcLength(maxContourFinal, true); // Adjust epsilon based on your needs 0.02 also good
    cv::approxPolyDP(maxContourFinal, approxFinal, epsilon, true);

    // Ensure the contour has exactly 4 vertices
    while (true) {
        cv::approxPolyDP(maxContourFinal, approxFinal, epsilon, true);
        if (approxFinal.size() == 4) {
            break;
        } else if (approxFinal.size() < 4) {
            epsilon -= 0.001; // Decrease epsilon to get more points
        } else {
            epsilon += 0.001; // Increase epsilon to reduce points
        }

        // Avoid infinite loop or invalid epsilon value
        if (epsilon <= 0) {
            std::cerr << "Failed to approximate to 4 corners." << std::endl;
            return;
        }
    }

    // Store the 4 corners in the edgePoints member variable
    this->edgePoints = approxFinal;

    // Draw the approximated contour in yellow
    cv::Scalar colorYellow(0, 255, 255); // Yellow color (BGR)
    cv::drawContours(result, std::vector<std::vector<cv::Point>>{approxFinal}, -1, colorYellow, 2); // Draw in yellow

    // Optionally draw the corners for visualization
    for (const auto& point : approxFinal) {
        cv::circle(result, point, 5, cv::Scalar(0, 0, 255), -1); // Red circles for corners
    }
    
    cv::imshow("4 edges", result);

    // Field based on edgePoints
    if (edgePoints.size() == 4) {
        // Draw lines between consecutive points
        for (size_t i = 0; i < edgePoints.size(); ++i) {
            cv::line(this->frame, edgePoints[i], edgePoints[(i + 1) % edgePoints.size()], cv::Scalar(0, 255, 255), 2);
        }
    }


    // Display the result
    cv::imshow("Field Detection", this->frame);
    cv::waitKey(0);
}

std::vector<cv::Point> fieldDetector::getEdgePoints() const {
    return this->edgePoints;
}

std::vector<cv::Point> fieldDetector::getFieldArea() const {
    /*if (edgePoints.size() == 4) {
        return cv::contourArea(edgePoints);
    } else {
        std::cerr << "Edge points do not form a quadrilateral." << std::endl;
        return 0;
    }*/
    return this->fieldArea;
}