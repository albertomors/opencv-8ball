#include "trajectoryProjection.h"
#include <cmath>
#include <algorithm>
#include <numeric>

cv::Point2f getIntersectionPoint(const cv::Vec4i& line1, const cv::Vec4i& line2) {
    float x1 = line1[0], y1 = line1[1], x2 = line1[2], y2 = line1[3];
    float x3 = line2[0], y3 = line2[1], x4 = line2[2], y4 = line2[3];

    float denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    if (denom == 0) {
        return cv::Point2f(-1, -1);  // Parallel lines, no intersection
    }

    float px = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / denom;
    float py = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / denom;

    return cv::Point2f(px, py);
}

trajectoryProjecter::trajectoryProjecter() {
}

void trajectoryProjecter::findLines(const cv::Mat& current_frame, const cv::Mat& roi) {
    cv::Mat blurred_roi;
    cv::GaussianBlur(roi, blurred_roi, cv::Size(17, 17), 0);
    cv::Mat canny;
    cv::Canny(blurred_roi, canny, 20, 50);
    cv::imshow("Canny", canny);

    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(canny, lines, 1, CV_PI / 180, 50, 200, 100);

    cv::Mat field_lines;
    current_frame.copyTo(field_lines);
    for (size_t i = 0; i < lines.size(); i++) {
        cv::Vec4i l = lines[i];
        cv::line(field_lines, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
    }

    // Find and highlight intersections
    std::vector<cv::Point2f> intersectionPoints;
    double thresholdDistance = 10.0;

    for (size_t i = 0; i < lines.size(); i++) {
        for (size_t j = i + 1; j < lines.size(); j++) {
            cv::Point2f intersection = getIntersectionPoint(lines[i], lines[j]);
            if (intersection.x >= 0 && intersection.y >= 0 && intersection.x < field_lines.cols && intersection.y < field_lines.rows) {
                bool tooClose = false;
                for (const auto& point : intersectionPoints) {
                    double dist = std::sqrt(std::pow(intersection.x - point.x, 2) + std::pow(intersection.y - point.y, 2));
                    if (dist < thresholdDistance) {
                        tooClose = true;
                        break;
                    }
                }
                if (!tooClose) {
                    intersectionPoints.push_back(intersection);
                    cv::circle(field_lines, intersection, 5, cv::Scalar(0, 255, 0), -1);
                }
            }
        }
    }

    this->corners = intersectionPoints;

    cv::imshow("Detected Lines", field_lines);
    cv::waitKey(0);
}

cv::Point2f calculateCenter(const std::vector<cv::Point2f>& corners) {
    cv::Point2f center(0, 0);
    for (const auto& corner : corners) {
        center += corner;
    }
    center *= (1.0 / corners.size());
    return center;
}

// Function to calculate the angle between the center and a corner
double calculateAngle(const cv::Point2f& center, const cv::Point2f& point) {
    return std::atan2(point.y - center.y, point.x - center.x);
}

// Function to calculate the Euclidean distance between two points
double calculateDistance(const cv::Point2f& p1, const cv::Point2f& p2) {
    return std::sqrt(std::pow(p1.x - p2.x, 2) + std::pow(p1.y - p2.y, 2));
}

// Function to sort corners based on their angle with respect to the center
std::vector<cv::Point2f> sortCornersByAngle(const std::vector<cv::Point2f>& corners, cv::Mat image) {
    cv::Point2f center = calculateCenter(corners);
    std::vector<std::pair<cv::Point2f, double>> cornersWithAngles;

    for (const auto& corner : corners) {
        double angle = calculateAngle(center, corner);
        cornersWithAngles.push_back(std::make_pair(corner, angle));
    }

    std::sort(cornersWithAngles.begin(), cornersWithAngles.end(), [](const std::pair<cv::Point2f, double>& a, const std::pair<cv::Point2f, double>& b) {
        return a.second < b.second;
    });

    std::vector<cv::Point2f> sortedCorners;
    for (size_t i = 0; i < cornersWithAngles.size(); ++i) {
        sortedCorners.push_back(cornersWithAngles[i].first);
        cv::putText(image, std::to_string(i), cornersWithAngles[i].first, cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 0, 0), 2);
    }
    cv::imshow("corners", image);
    cv::waitKey(0);

    return sortedCorners;
}

void trajectoryProjecter::projectBalls(const cv::Mat& frame, const std::vector<cv::Point2f>& balls) {
    // Define the dimensions of the bird's-eye view
    cv::Size birdEyeViewSize(800, 400); // Adjust the size as needed

    // Define the destination points for the perspective transform
    std::vector<cv::Point2f> dstCorners = {
        cv::Point2f(0, 0),
        cv::Point2f(birdEyeViewSize.width, 0),
        cv::Point2f(birdEyeViewSize.width, birdEyeViewSize.height),
        cv::Point2f(0, birdEyeViewSize.height)
    };

    // Sort corners by angle
    this->corners = sortCornersByAngle(this->corners, frame);

    // Calculate distances between corners to determine edge lengths
    std::vector<double> edgeLengths;
    for (size_t i = 0; i < 4; ++i) {
        size_t nextIdx = (i + 1) % 4;
        edgeLengths.push_back(calculateDistance(this->corners[i], this->corners[nextIdx]));
    }

    // Find indices of the two longest edges
    std::vector<size_t> sortedIndices(4);
    std::iota(sortedIndices.begin(), sortedIndices.end(), 0);
    std::sort(sortedIndices.begin(), sortedIndices.end(), [&edgeLengths](size_t a, size_t b) {
        return edgeLengths[a] > edgeLengths[b];
    });

    // Colors for longer and shorter edges
    cv::Scalar longColor(0, 255, 0);  // Green for longer sides
    cv::Scalar shortColor(0, 0, 255); // Red for shorter sides

    cv::Mat field_lines = frame.clone();

    // Draw the two longest edges
    cv::line(field_lines, this->corners[sortedIndices[0]], this->corners[sortedIndices[1]], longColor, 2);
    // Draw the two shorter edges
    cv::line(field_lines, this->corners[sortedIndices[2]], this->corners[sortedIndices[3]], shortColor, 2);

    cv::imshow("Field with Long and Short Sides", field_lines);
    cv::waitKey(0);

    // Get the perspective transform matrix
    cv::Mat perspectiveMatrix = cv::getPerspectiveTransform(this->corners, dstCorners);

    // Transform the ball positions
    std::vector<cv::Point2f> birdEyeBallPositions;
    cv::perspectiveTransform(balls, birdEyeBallPositions, perspectiveMatrix);

    // Create a black background image
    cv::Mat birdEyeView = cv::Mat::zeros(birdEyeViewSize, CV_8UC3);

    // Draw the balls on the bird's-eye view
    for (const auto& pos : birdEyeBallPositions) {
        cv::circle(birdEyeView, pos, 5, cv::Scalar(0, 0, 255), -1);
    }

    std::cout << "Bird's Eye View Ball Positions:" << std::endl;
    for (const auto& pos : birdEyeBallPositions) {
        std::cout << "Ball Position: (" << pos.x << ", " << pos.y << ")" << std::endl;
    }

    // Display the bird's-eye view
    cv::imshow("Bird's Eye View", birdEyeView);
    cv::waitKey(0);
}
