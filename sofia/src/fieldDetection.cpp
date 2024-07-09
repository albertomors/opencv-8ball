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
    int threshold = 50; // Adjust this threshold as needed
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
    cv::imshow("prova", mask_colour);
    cv::imwrite("test.jpg", mask_colour);

    // Convert to grayscale
    cv::Mat gray;
    cv::cvtColor(this->frame, gray, cv::COLOR_BGR2GRAY);
    
    // Apply GaussianBlur to reduce noise and improve edge detection
    cv::GaussianBlur(gray, gray, cv::Size(5, 5), 0);

    // Edge detection using Canny
    cv::Mat edges;
    cv::Canny(mask_colour, edges, 50, 150);
    cv::imshow("Canny", edges);

    // Hough Lines Probabilistic
    std::vector<cv::Vec4i> linesP;
    cv::HoughLinesP(edges, linesP, 1, CV_PI / 180, 100, 50, 2); //resolution[pixel],resolution[rad],min length, max gap, votes

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
        cv::line(result, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0, 255, 0), 3, cv::LINE_AA);
    }

    cv::imshow("Mask HoughP", result); //DEBUG


    //NON FA QUELLO CHE VOGLIO!!!!!
    //try rough kmeans segmentation for macro areas and put togheter with contours maybe and then hough lines


    // Display the result
    //cv::imshow("Final Field Result", result);
    //cv::imshow("Area Field", this->fieldArea);
    cv::waitKey(0);
}

std::vector<cv::Point> fieldDetector::getEdgePoints() const {
    return this->edgePoints;
}

std::vector<cv::Point> fieldDetector::getFieldArea() const {
    return this->fieldArea;
}