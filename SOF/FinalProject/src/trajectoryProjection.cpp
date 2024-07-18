#include "trajectoryProjection.h"

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

// Function to calculate centroid of a set of points
    cv::Point2f computeCentroid(const std::vector<cv::Point2f>& points) {
        cv::Point2f centroid(0, 0);
        for (const auto& pt : points) {
            centroid += pt;
        }
        centroid *= (1.0 / points.size());
        return centroid;
    }

// Function to sort corners in clockwise order
    std::vector<cv::Point2f> sortCornersClockwise(std::vector<cv::Point2f>& corners) {
        // Calculate centroid
        cv::Point2f centroid = computeCentroid(corners);

        // Sort corners based on the angle they make with the centroid
        std::sort(corners.begin(), corners.end(), [centroid](const cv::Point2f& a, const cv::Point2f& b) {
            return std::atan2(a.y - centroid.y, a.x - centroid.x) < std::atan2(b.y - centroid.y, b.x - centroid.x);
        });

        return corners;
    }

trajectoryProjecter::trajectoryProjecter() {
}

void trajectoryProjecter::findLines(const cv::Mat& current_frame, const cv::Mat& roi){

    cv::Mat blurred_roi;
    cv::GaussianBlur(roi, blurred_roi, cv::Size(17, 17), 0);
    cv::Mat canny;
    cv::Canny(blurred_roi, canny, 20, 50);
    cv::imshow("Canny", canny);

    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(canny, lines, 1, 3*CV_PI/180, 50, 200, 1000);

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

    // Ensure corners are sorted clockwise
    this->corners = sortCornersClockwise(this->corners);

    // Draw numbered corners on the frame
    for (int i = 0; i < this->corners.size(); ++i) {
        cv::putText(frame, std::to_string(i + 1), this->corners[i], cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255), 2);
        cv::circle(frame, this->corners[i], 5, cv::Scalar(0, 255, 0), -1);
    }

    // Compute perspective transform matrix
    cv::Mat perspectiveMatrix = cv::getPerspectiveTransform(this->corners, dstCorners);

    // Transform reference points to get original frame coordinates
    std::vector<cv::Point2f> transformedCorners(4);
    cv::perspectiveTransform(dstCorners, transformedCorners, perspectiveMatrix);

    // Compute diagonal lengths of the transformed rectangle
    double diagonal1Length = cv::norm(transformedCorners[2] - transformedCorners[0]); // Top-left to bottom-right
    double diagonal2Length = cv::norm(transformedCorners[3] - transformedCorners[1]); // Top-right to bottom-left

    // Determine table orientation
    bool isVertical = (diagonal1Length > diagonal2Length);
    std::string orientationText = isVertical ? "Vertical" : "Horizontal";

    // Write the orientation on the frame
    cv::putText(frame, orientationText, cv::Point(50, 50), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);

    // Display the annotated frame
    cv::imshow("Corners and Orientation", frame);
    cv::waitKey(0);

    // Get the perspective transform matrix for bird's-eye view
    perspectiveMatrix = cv::getPerspectiveTransform(this->corners, dstCorners);

    // Transform the ball positions (adjust for vertical table)
    std::vector<cv::Point2f> birdEyeBallPositions;
    if (isVertical) {
        std::rotate(this->corners.begin(), this->corners.begin() + 1, this->corners.end());
        perspectiveMatrix = cv::getPerspectiveTransform(this->corners, dstCorners);
    }

    cv::perspectiveTransform(balls, birdEyeBallPositions, perspectiveMatrix);

    // Create a black background image for bird's-eye view
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