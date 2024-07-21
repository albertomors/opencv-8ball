/*
    AUTHOR: Fresco Eleonora
    DATE: 2024-07-21
    FILE: trajectoryProjection.cpp
    DESCRIPTION: Implementation of the trajectory projection algorithm for billiard balls onto a table minimap. This module computes the bird's-eye view of the balls and their trajectories using perspective transforms.

    CLASSES:
    - class trajectoryProjecter: Class for projecting the balls' trajectory onto a bird eye view minimap.

    FUNCTIONS:
    - cv::Point2f computeCentroid(const std::vector<cv::Point2f>& points): Computes the centroid of a given set of points.
    - std::vector<cv::Point2f> sortCornersClockwise(std::vector<cv::Point2f>& corners): Sorts corners in clockwise order based on their angle from the centroid.
    - trajectoryProjecter::trajectoryProjecter(): Constructor for the trajectoryProjecter class.
    - void trajectoryProjecter::projectBalls(const cv::Mat& frame, const std::vector<cv::Point2f>& balls, const std::vector<std::vector<cv::Point2f>>& trajectories, const std::vector<int>& id_balls, const std::vector<cv::Point2f>& corners): Projects ball positions and trajectories onto a table minimap and displays the result.

    NOTES:
    - The table minimap image should be placed in the "../res/" directory.
    - The function `projectBalls` overlays the table minimap image onto the bottom-left corner of the input frame.
    - Balls and their trajectories are drawn on the minimap image with colors assigned based on their IDs.
*/

#include "trajectoryProjection.h"

cv::Point2f computeCentroid(const std::vector<cv::Point2f>& points) {
    cv::Point2f centroid(0, 0);
    for (const auto& pt : points) {
        centroid += pt;
    }
    centroid *= (1.0 / points.size());
    return centroid;
}

std::vector<cv::Point2f> sortCornersClockwise(std::vector<cv::Point2f>& corners) {
    // Calculate centroid
    cv::Point2f centroid = computeCentroid(corners);

    // Sort corners based on the angle they make with the centroid
    std::sort(corners.begin(), corners.end(), [centroid](const cv::Point2f& a, const cv::Point2f& b) {
        return std::atan2(a.y - centroid.y, a.x - centroid.x) < std::atan2(b.y - centroid.y, b.x - centroid.x);
    });

    return corners;
}

// Constructor of the class
trajectoryProjecter::trajectoryProjecter() {}

void trajectoryProjecter::projectBalls(const cv::Mat& frame, const std::vector<cv::Point2f>& balls, const std::vector<std::vector<cv::Point2f>>& trajectories, const std::vector<int>& id_balls, std::vector<cv::Point2f>& corners) {
    // Load the table minimap image
    cv::Mat tableImage = cv::imread("../res/table.png", cv::IMREAD_UNCHANGED);

    if (tableImage.empty()) {
        std::cerr << "Error: Unable to load table image." << std::endl;
        return;
    }

    // Define the size for the table image on the frame
    cv::Size tableImageSize(300, 150);

    // Adjust the translation onto the minimap with respect to the chosen base image
    int tableBorderWidth_horizontal = 10;
    int tableBorderWidth_vertical = 15;

    std::vector<cv::Point2f> dstCorners = {
        cv::Point2f(tableBorderWidth_horizontal, tableBorderWidth_vertical),
        cv::Point2f(tableImageSize.width - tableBorderWidth_horizontal, tableBorderWidth_vertical),
        cv::Point2f(tableImageSize.width - tableBorderWidth_horizontal, tableImageSize.height - tableBorderWidth_vertical),
        cv::Point2f(tableBorderWidth_horizontal, tableImageSize.height - tableBorderWidth_vertical)
    };

    // Resize the table minimap image
    cv::Mat resizedTableImage;
    cv::resize(tableImage, resizedTableImage, tableImageSize);

    // Ensure resizedTableImage has 3 channels
    if (resizedTableImage.channels() == 4) {
        cv::cvtColor(resizedTableImage, resizedTableImage, cv::COLOR_BGRA2BGR);
    }

    // Define the region where the table minimap image will be placed (bottom-left corner)
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
    corners = sortCornersClockwise(corners);

    // Compute perspective transform matrix
    cv::Mat perspectiveMatrix = cv::getPerspectiveTransform(corners, dstCorners);

    // Transform reference points to get original frame coordinates
    std::vector<cv::Point2f> transformedCorners(4);
    cv::perspectiveTransform(dstCorners, transformedCorners, perspectiveMatrix);

    // Compute diagonal lengths of the transformed rectangle
    double diagonal1Length = cv::norm(transformedCorners[2] - transformedCorners[0]); // Top-left to bottom-right
    double diagonal2Length = cv::norm(transformedCorners[3] - transformedCorners[1]); // Top-right to bottom-left

    // Determine table orientation
    bool isVertical = (diagonal1Length > diagonal2Length);
    std::string orientationText = isVertical ? "Vertical" : "Horizontal";

    // Adjust perspective matrix for vertical table
    if (isVertical) {
        std::rotate(corners.begin(), corners.begin() + 1, corners.end());
        perspectiveMatrix = cv::getPerspectiveTransform(corners, dstCorners);
    }

    // Transform the ball positions
    std::vector<cv::Point2f> birdEyeBallPositions;

    try {
        cv::perspectiveTransform(balls, birdEyeBallPositions, perspectiveMatrix);
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

    // Define a color map for different IDs
    std::map<int, cv::Scalar> colorMap = {
        {1, cv::Scalar(255, 255, 255)}, // White for ID 1
        {2, cv::Scalar(0, 0, 0)},       // Black for ID 2
        {3, cv::Scalar(0, 0, 255)},     // Red for ID 3
        {4, cv::Scalar(255, 0, 0)},     // Blue for ID 4
    };

    // Draw the balls on the resized minimap table image before overlaying
    for (size_t i = 0; i < birdEyeBallPositions.size(); ++i) {
        int id = id_balls[i];
        // Ensure the ID exists in the color map
        if (colorMap.find(id) != colorMap.end()) {
            cv::Scalar color = colorMap[id]; // Assign color based on ID
            cv::Point2f pos = birdEyeBallPositions[i];
            cv::circle(resizedTableImage, pos, 5, color, -1);
        } else {
            std::cerr << "Warning: ID " << id << " not found in color map!" << std::endl;
        }
    }

    // Draw the trajectories on the bird's-eye view
    for (const auto& trajectory : birdEyeTrajectories) {
        for (size_t j = 1; j < trajectory.size(); ++j) {
            cv::line(resizedTableImage, trajectory[j - 1], trajectory[j], cv::Scalar(0, 255, 255), 2);
        }
    }

    // Create a copy of the frame to avoid modifying the original frame
    cv::Mat frameWithOverlay = frame.clone();

    // Place the resized table image on the bottom-left corner of the frame
    resizedTableImage.copyTo(frameWithOverlay(roi));
    cv::namedWindow("RISULTATO PROJECTION");
    cv::imshow("RISULTATO PROJECTION", frameWithOverlay);
}
