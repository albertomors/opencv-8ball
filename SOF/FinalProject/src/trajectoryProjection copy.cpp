/*
AUTHOR: Girardello Sofia 
*/

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

/*

// Function to sort corners in the order: top-left, top-right, bottom-right, bottom-left
std::vector<cv::Point2f> sortCorners(const std::vector<cv::Point2f>& corners) {
    // Calculate the centroid of the corners
    cv::Point2f centroid(0, 0);
    for (const auto& pt : corners) {
        centroid += pt;
    }
    centroid *= (1.0 / corners.size());

    // Separate points into top and bottom halves
    std::vector<cv::Point2f> top, bottom;
    for (const auto& pt : corners) {
        if (pt.y < centroid.y)
            top.push_back(pt);
        else
            bottom.push_back(pt);
    }

    // Sort top points by x-coordinate
    std::sort(top.begin(), top.end(), [](const cv::Point2f& a, const cv::Point2f& b) {
        return a.x < b.x;
    });

    // Sort bottom points by x-coordinate
    std::sort(bottom.begin(), bottom.end(), [](const cv::Point2f& a, const cv::Point2f& b) {
        return a.x < b.x;
    });

    // Combine sorted points: top-left, top-right, bottom-right, bottom-left
    std::vector<cv::Point2f> sortedCorners = { top[0], top[1], bottom[1], bottom[0] };

    return sortedCorners;
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

    std::vector<cv::Point2f> sortCornersRowByRow(const std::vector<cv::Point2f>& corners) {
        std::vector<cv::Point2f> sortedCorners = corners;

        // Sort corners based on y-coordinate first, then x-coordinate
        std::sort(sortedCorners.begin(), sortedCorners.end(), [](const cv::Point2f& a, const cv::Point2f& b) {
            if (std::abs(a.y - b.y) < 1e-5) {
                return a.x < b.x; // Sort by x if y coordinates are approximately equal
            }
            return a.y < b.y; // Sort by y otherwise
        });

        return sortedCorners;
    }

    // Function to check if the longer side is vertical
    bool isVerticalLonger(std::vector<cv::Point2f>  corners) {
        // Calculate lengths of sides defined by corners
        float dx1 = corners[1].x - corners[0].x;
        float dx2 = corners[2].x - corners[0].x;
        float dy1 = corners[1].y - corners[0].y;
        float dy2 = corners[2].y - corners[0].y;

        float length_horizontal = std::sqrt(dx1 * dx1 + dy1 * dy1);
        float length_vertical = std::sqrt(dx2 * dx2 + dy2 * dy2);

        // Determine if longer side is vertical
        return length_vertical > length_horizontal;
    }

    // Function to rotate ball positions by 90 degrees clockwise
    void rotatePositions(std::vector<cv::Point2f>& positions) {
        for (auto& pos : positions) {
            float x = pos.x;
            pos.x = pos.y;
            pos.y = x;
        }
    }

/*

//prova per warp 
    cv::Mat canny;
    cv::Canny(ROI, canny, 20, 50);
    cv::imshow("Canny", canny);

    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(canny, lines, 0.5, CV_PI/360, 50, 50, 700);

    cv::Mat field_lines;
    currentFrame.copyTo(field_lines);
    for (size_t i = 0; i < lines.size(); i++) {
        cv::Vec4i l = lines[i];
        cv::line(field_lines, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0, 0, 255), 3, cv::LINE_AA);
    }


    // Display the result
    cv::imshow("Detected Lines", field_lines);
    cv::waitKey(0);

*/
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


/*
void trajectoryProjecter::projectBalls(const cv::Mat& frame, const std::vector<cv::Point2f>& balls){

    // Define the dimensions of the bird's-eye view
    cv::Size birdEyeViewSize(800, 400); // Adjust the size as needed

    // Define the destination points for the perspective transform
    std::vector<cv::Point2f> dstCorners = {
        cv::Point2f(0, 0),
        cv::Point2f(birdEyeViewSize.width, 0),
        cv::Point2f(birdEyeViewSize.width, birdEyeViewSize.height),
        cv::Point2f(0, birdEyeViewSize.height)
    };

    // Get the perspective transform matrix
    //Top-left - Top-right - Bottom-right - Bottom-left
    this->corners = sortCorners(this->corners);
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


void trajectoryProjecter::projectBalls(const cv::Mat& frame, const std::vector<cv::Point2f>& balls){

    int outputWidth = 800;
    int outputHeight = 400;
    cv::Size birdEyeViewSize(800, 400); // Adjust the size as needed

    // Calculate the bounding box of the input points
    cv::Rect boundingBox = cv::boundingRect(this->corners);

    // Determine if the longer side is horizontal or vertical
    bool isHorizontal = (boundingBox.width >= boundingBox.height);

    // Define the output corners (bird's-eye view rectangle)
    std::vector<cv::Point2f> dstCorners;
    if (isHorizontal) {
        dstCorners = {
            cv::Point2f(0, 0),
            cv::Point2f(outputWidth - 1, 0),
            cv::Point2f(outputWidth - 1, outputHeight - 1),
            cv::Point2f(0, outputHeight - 1)
        };
    } else {
        dstCorners = {
            cv::Point2f(0, 0),
            cv::Point2f(outputHeight - 1, 0),
            cv::Point2f(outputHeight - 1, outputWidth - 1),
            cv::Point2f(0, outputWidth - 1)
        };
    }

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

        // Ensure corners are sorted 
        this->corners = sortCornersRowByRow(this->corners);

        // Draw numbered corners on bird's-eye view
            for (int i = 0; i < this->corners.size(); ++i) {
                cv::putText(frame, std::to_string(i + 1), this->corners[i], cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255), 2);
                cv::circle(frame, corners[i], 5, cv::Scalar(0, 255, 0), -1);
            }
            cv::imshow("numbers", frame);

        // Check if longer side is vertical
        if (isVerticalLonger(this->corners)) {
        //if (0) {
            std::cout <<"vertical longer";
            // Rotate ball positions by 90 degrees clockwise
            std::vector<cv::Point2f> rotatedBalls = balls;
            rotatePositions(rotatedBalls);
            
            // Transform the rotated ball positions
            std::vector<cv::Point2f> birdEyeBallPositions;
            cv::Mat perspectiveMatrix = cv::getPerspectiveTransform(this->corners, dstCorners);
            cv::perspectiveTransform(rotatedBalls, birdEyeBallPositions, perspectiveMatrix);

            // Create a black background image
            cv::Mat birdEyeView = cv::Mat::zeros(birdEyeViewSize, CV_8UC3);

            // Draw the balls on the bird's-eye view
            for (const auto& pos : birdEyeBallPositions) {
                cv::circle(birdEyeView, pos, 5, cv::Scalar(0, 0, 255), -1);
            }

            // Display the bird's-eye view
            cv::imshow("Bird's Eye View", birdEyeView);
            cv::waitKey(0);
        } else {
            std::cout <<"horizontal longer";
            // Longer side is horizontal, proceed normally
            std::vector<cv::Point2f> birdEyeBallPositions;
            cv::Mat perspectiveMatrix = cv::getPerspectiveTransform(corners, dstCorners);
            cv::perspectiveTransform(balls, birdEyeBallPositions, perspectiveMatrix);

            // Create a black background image
            cv::Mat birdEyeView = cv::Mat::zeros(birdEyeViewSize, CV_8UC3);

            // Draw the balls on the bird's-eye view
            for (const auto& pos : birdEyeBallPositions) {
                cv::circle(birdEyeView, pos, 5, cv::Scalar(0, 0, 255), -1);
            }

            // Display the bird's-eye view
            cv::imshow("Bird's Eye View", birdEyeView);
            cv::waitKey(0);
        }
    }
*/

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