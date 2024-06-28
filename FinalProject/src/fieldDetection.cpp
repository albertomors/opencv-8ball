/*AUTHOR: Fresco Eleonora */
#include "fieldDetection.h"

// Constructor to initialize the frame
fieldDetector::fieldDetector(const cv::Mat& currentFrame) {
    this->frame = currentFrame.clone();  // Ensure a deep copy
}

// Simplified version
cv::Vec2f findIntersection(cv::Vec2f line1, cv::Vec2f line2) {
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
}

bool isPointOnLine(const cv::Point2f& pt, const cv::Vec2f& line, float epsilon = 1.0) {
    float rho = line[0];
    float theta = line[1];
    float x = pt.x;
    float y = pt.y;
    float distance = std::abs(x * cos(theta) + y * sin(theta) - rho);
    return distance < epsilon;
}

double calculateRoiArea(const std::vector<cv::Point2f>& points) {
    if (points.size() != 4) {
        return 0.0; // Not a valid quadrilateral
    }
    cv::Point2f p0 = points[0];
    cv::Point2f p1 = points[1];
    cv::Point2f p2 = points[2];
    cv::Point2f p3 = points[3];
    return 0.5 * std::abs(p0.x * p1.y + p1.x * p2.y + p2.x * p3.y + p3.x * p0.y - (p1.x * p0.y + p2.x * p1.y + p3.x * p2.y + p0.x * p3.y));
}

/*void drawSegmentedLines(cv::Mat& image, const std::vector<cv::Vec2f>& lines, const std::vector<cv::Point2f>& intersections) {
    for (const auto& line : lines) {
        std::vector<cv::Point2f> pointsOnLine;
        for (const auto& pt : intersections) {
            if (isPointOnLine(pt, line)) {
                pointsOnLine.push_back(pt);
            }
        }

        // Sort points along the line
        std::sort(pointsOnLine.begin(), pointsOnLine.end(), [&line](const cv::Point2f& a, const cv::Point2f& b) {
            float rho = line[0];
            float theta = line[1];
            return (a.x * cos(theta) + a.y * sin(theta)) < (b.x * cos(theta) + b.y * sin(theta));
        });

        // Draw segments between consecutive points
        for (size_t i = 0; i < pointsOnLine.size() - 1; ++i) {
            cv::line(image, pointsOnLine[i], pointsOnLine[i + 1], cv::Scalar(0, 0, 255), 2);
        }
    }
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


void fieldDetector::detectField() {

    cv::Mat result = this->frame.clone();

    // Convert to grayscale
    cv::Mat gray;
    cv::cvtColor(this->frame, gray, cv::COLOR_BGR2GRAY);
    
    // Apply GaussianBlur to reduce noise and improve edge detection
    cv::GaussianBlur(gray, gray, cv::Size(5, 5), 0);

    // Edge detection using Canny
    cv::Mat edges;
    cv::Canny(gray, edges, 50, 150);

    // Detect contours to find potential table boundaries
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(edges, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);


    cv::Mat mask = cv::Mat::zeros(this->frame.size(), CV_8UC1);

    //contour
    
    // Find the contour with the largest area (assuming it's the table)
    double maxArea = 0;
    std::vector<cv::Point> maxContour;
    for (const auto& contour : contours) {
        double area = cv::contourArea(contour);
        if (area > maxArea) {
            maxArea = area;
            maxContour = contour;
        }
    }
    // Approximate the contour to a polygon (rectangle in this case)
    std::vector<cv::Point> approx;
    cv::approxPolyDP(maxContour, approx, cv::arcLength(maxContour, true) * 0.02, true);

    // Store the detected corner points --REMOVE
    /*this->edgePoints = approx;
    // Optionally, you can also store the convex hull of the detected points to define the field area
    if (!approx.empty()) {
        cv::convexHull(approx, this->fieldArea);
    }*/

    // Draw contours and corners on the frame (for visualization)--REMOVE
    /*cv::drawContours(this->frame, contours, -1, cv::Scalar(0, 255, 255), 2);
    for (size_t i = 0; i < approx.size(); i++) {
        cv::circle(this->frame, approx[i], 5, cv::Scalar(0, 0, 255), -1); // Red circles at detected corners
    }*/

    //REMOVE
    cv::drawContours(mask, contours, -1, cv::Scalar(255), cv::FILLED);

    //DEBUG
    cv::imshow("Contours Mask", mask);
    //cv::imshow("Countours Field", this->frame);

    // Hough Lines Probabilistic
    std::vector<cv::Vec4i> linesP;
    cv::HoughLinesP(mask, linesP, 1, CV_PI / 180, 50, 50, 3); //resolution[pixel],resolution[rad],min length, max gap, votes

    // Keep only strongest lines
    double minLineLength = 50.0;
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
        //cv::line(result, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(255, 0, 0), 3, cv::LINE_AA);
        cv::line(mask, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(255, 0, 0), 3, cv::LINE_AA);
    }

    cv::imshow("Mask HoughP", mask); //DEBUG

    // Hough Lines
    std::vector<cv::Vec2f> lines;
    cv::HoughLines(mask, lines, 1, CV_PI / 180, 300); 

    // Draw detected lines on the frame (for visualization)--REMOVE
    for (size_t i = 0; i < lines.size(); i++) {
        float rho = lines[i][0];
        float theta = lines[i][1];
        cv::Point pt1, pt2;
        double a = std::cos(theta), b = std::sin(theta);
        double x0 = a * rho, y0 = b * rho;
        pt1.x = cvRound(x0 + 1000 * (-b));
        pt1.y = cvRound(y0 + 1000 * (a));
        pt2.x = cvRound(x0 - 1000 * (-b));
        pt2.y = cvRound(y0 - 1000 * (a));
        cv::line(mask, pt1, pt2, cv::Scalar(0, 255, 255), 2, cv::LINE_AA);
        //cv::line(result, pt1, pt2, cv::Scalar(0, 255, 255), 2, cv::LINE_AA);
    }
    cv::imshow("Mask Hough", mask); //DEBUG

    // Vector to store intersections
    std::vector<cv::Vec2f> intersections;

    // Find intersections among the lines
    for (size_t i = 0; i < lines.size(); ++i) {
        for (size_t j = i + 1; j < lines.size(); ++j) {
            cv::Vec2f intersection = findIntersection(lines[i], lines[j]);
            if (intersection != cv::Vec2f(-1, -1)) {
                intersections.push_back(intersection);
            }
        }
    }
    // Draw intersection points --REMOVE
    /*for (size_t i = 0; i < intersections.size(); ++i) {
        cv::Point intersectionPoint(intersections[i][0], intersections[i][1]);
        cv::circle(result, intersectionPoint, 5, cv::Scalar(0, 0, 255), -1); // Red filled circle
    }*/


    //NON FA QUELLO CHE VOGLIO!!!!!
    //try rough kmeans segmentation for macro areas and put togheter with contours maybe and then hough lines


    // Display the result
    cv::imshow("Final Field Result", result);
    //cv::imshow("Area Field", this->fieldArea);
    cv::waitKey(0);
}

std::vector<cv::Point> fieldDetector::getEdgePoints() const {
    return this->edgePoints;
}

std::vector<cv::Point> fieldDetector::getFieldArea() const {
    return this->fieldArea;
}