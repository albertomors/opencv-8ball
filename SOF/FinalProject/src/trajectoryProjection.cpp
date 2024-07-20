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

void trajectoryProjecter::findLines(const cv::Mat& current_frame, std::vector<cv::Point>& hull){

    cv::Mat origin_frame = current_frame.clone();  // Store the original frame

        // Create an empty ROI
        cv::Mat roi = cv::Mat::zeros(origin_frame.size(), CV_8UC1);
        
        // Fill the ROI with the convex hull
        cv::fillConvexPoly(roi, hull, cv::Scalar(255));
        cv::imshow("ROI", roi);

    cv::Mat blurred_roi;
    cv::GaussianBlur(roi, blurred_roi, cv::Size(25, 25), 0);
    cv::Mat canny;
    cv::Canny(blurred_roi, canny, 20, 50);
    cv::imshow("Canny", canny);

    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(canny, lines, 2, 3*CV_PI/180, 80, 200, 1000);

    cv::Mat field_lines;
    current_frame.copyTo(field_lines);
    for (size_t i = 0; i < lines.size(); i++) {
        cv::Vec4i l = lines[i];
        cv::line(field_lines, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
    }

    // Find and highlight intersections
    std::vector<cv::Point2f> intersectionPoints;
    double thresholdDistance = 20.0;

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
    //cv::waitKey(0);
}


void trajectoryProjecter::projectBalls(const cv::Mat& frame, const std::vector<cv::Point2f>& balls, const std::vector<std::vector<cv::Point2f>>& trajectories, const std::vector<int>& id_balls) {
    // Define the dimensions of the bird's-eye view
    cv::Size birdEyeViewSize(800, 400); // Adjust the size as needed

    // Load the table image
    cv::Mat tableImage = cv::imread("table.png", cv::IMREAD_UNCHANGED);

    if (tableImage.empty()) {
        std::cerr << "Error: Unable to load table image." << std::endl;
        return;
    }

    // Define the size for the table image on the frame
    cv::Size tableImageSize(300, 150); 

    // Define the destination points for the perspective transform
    /*std::vector<cv::Point2f> dstCorners = {
        cv::Point2f(0, 0),
        cv::Point2f(tableImageSize.width, 0),
        cv::Point2f(tableImageSize.width, tableImageSize.height),
        cv::Point2f(0, tableImageSize.height)
    };*/

    int tableBorderWidth_horizontal = 10;
    int tableBorderWidth_vertical = 15;
    std::vector<cv::Point2f> dstCorners = {
        cv::Point2f(tableBorderWidth_horizontal, tableBorderWidth_vertical),
        cv::Point2f(tableImageSize.width-tableBorderWidth_horizontal, tableBorderWidth_vertical),
        cv::Point2f(tableImageSize.width-tableBorderWidth_horizontal, tableImageSize.height-tableBorderWidth_vertical),
        cv::Point2f(tableBorderWidth_horizontal, tableImageSize.height-tableBorderWidth_vertical)
    };

    // Resize the table image
    cv::Mat resizedTableImage;
    cv::resize(tableImage, resizedTableImage, tableImageSize);

    // Ensure resizedTableImage has 3 channels
    if (resizedTableImage.channels() == 4) {
        cv::cvtColor(resizedTableImage, resizedTableImage, cv::COLOR_BGRA2BGR);
    }

    // Define the region where the table image will be placed (bottom-left corner)
    cv::Rect roi(0, frame.rows - tableImageSize.height, tableImageSize.width, tableImageSize.height);

    // Check if ROI is within the frame dimensions
    if (roi.x < 0 || roi.y < 0 || roi.x + roi.width > frame.cols || roi.y + roi.height > frame.rows) {
        std::cerr << "Error: ROI is outside the frame dimensions." << std::endl;
        return;
    }

    // Ensure the resized table image fits into the ROI
    if (resizedTableImage.size() != cv::Size(roi.width, roi.height)) {
        std::cerr << "Error: Resized table image size does not match the ROI size." << std::endl;
        return;
    }

    // Ensure corners are sorted clockwise
    this->corners = sortCornersClockwise(this->corners);

    // Draw numbered corners on the frame
    for (int i = 0; i < this->corners.size(); ++i) {
        cv::putText(frame, std::to_string(i + 1), this->corners[i], cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255), 2);
        cv::circle(frame, this->corners[i], 5, cv::Scalar(0, 255, 0), -1);
    }

    // Compute perspective transform matrix
    cv::Mat perspectiveMatrix = cv::getPerspectiveTransform(this->corners, dstCorners);

    // Debug info
    //printMatInfo(perspectiveMatrix, "Perspective Matrix");

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
    //cv::waitKey(0);

    // Adjust perspective matrix for vertical table
    if (isVertical) {
        std::rotate(this->corners.begin(), this->corners.begin() + 1, this->corners.end());
        perspectiveMatrix = cv::getPerspectiveTransform(this->corners, dstCorners);
    }

    // Debug info
    //printMatInfo(perspectiveMatrix, "Adjusted Perspective Matrix");

    // Transform the ball positions
    std::vector<cv::Point2f> birdEyeBallPositions;
    std::vector<cv::Point2f> birdEyeTrajectoryPoints;

    try {
        cv::perspectiveTransform(balls, birdEyeBallPositions, perspectiveMatrix);
        std::cout << "Bird's Eye View Ball Positions:" << std::endl;
        for (const auto& pos : birdEyeBallPositions) {
            std::cout << "Ball Position: (" << pos.x << ", " << pos.y << ")" << std::endl;
        }
    } catch (const cv::Exception& e) {
        std::cerr << "Error in perspectiveTransform for balls: " << e.what() << std::endl;
    }

    // Transform the trajectories
    std::vector<std::vector<cv::Point2f>> birdEyeTrajectories(trajectories.size());
    for (size_t i = 0; i < trajectories.size(); ++i) {
        try {
            cv::perspectiveTransform(trajectories[i], birdEyeTrajectories[i], perspectiveMatrix);
        } catch (const cv::Exception& e) {
            std::cerr << "Error in perspectiveTransform for trajectory " << i << ": " << e.what() << std::endl;
        }
    }

    // Create a black background image for bird's-eye view
    cv::Mat birdEyeView = cv::Mat::zeros(birdEyeViewSize, CV_8UC3);

    // Define a color map for different IDs
    std::map<int, cv::Scalar> colorMap = {
        {1, cv::Scalar(255, 255, 255)}, // White for ID 1
        {2, cv::Scalar(0, 0, 0)}, // Black for ID 2
        {3, cv::Scalar(0, 0, 255)}, // Red for ID 3
        {4, cv::Scalar(255, 0, 0)}, // Blue for ID 4
    };

    // Draw the balls and trajectories on the resized table image before overlaying
    for (size_t i = 0; i < birdEyeBallPositions.size(); ++i) {
        int id = id_balls[i];
        std::cout << id << std::endl;
        // Ensure the ID exists in the color map
        if (colorMap.find(id) != colorMap.end()) {
            cv::Scalar color = colorMap[id];  // Assign color based on ID
            cv::Point2f pos = birdEyeBallPositions[i];
            cv::circle(resizedTableImage, pos, 5, color, -1);
        } else {
            std::cerr << "Warning: ID " << id << " not found in color map!" << std::endl;
        }
    }
    // Draw the balls on the bird's-eye view
    /*for (const auto& pos : birdEyeBallPositions) {
        cv::circle(resizedTableImage, pos, 5, cv::Scalar(0, 0, 255), -1);
    }*/

    // Draw the trajectories on the bird's-eye view
    for (const auto& trajectory : birdEyeTrajectories) {
        for (size_t j = 1; j < trajectory.size(); ++j) {
            cv::line(resizedTableImage, trajectory[j - 1], trajectory[j], cv::Scalar(255, 255, 255), 2);
        }
    }

    std::cout << "Bird's Eye View Ball Positions and Trajectories:" << std::endl;
    for (const auto& pos : birdEyeBallPositions) {
        std::cout << "Ball Position: (" << pos.x << ", " << pos.y << ")" << std::endl;
    }

    // Display the bird's-eye view
    //cv::imshow("Bird's Eye View", birdEyeView);
    //cv::waitKey(0);

    // Create a copy of the frame to avoid modifying the original frame
    cv::Mat frameWithOverlay = frame.clone();

    // Place the resized table image on the bottom-left corner of the frame
    resizedTableImage.copyTo(frameWithOverlay(roi));
    cv::imshow("Prova", frameWithOverlay);
}