#include "trajectoryProjection.h"

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

//const std::vector<cv::Point2f>& corners
trajectoryProjecter::trajectoryProjecter() {
    //this->corners = corners.copy();
}

void trajectoryProjecter::projectBalls(const cv::Mat& frame, const std::vector<cv::Point2f>& balls, const std::vector<std::vector<cv::Point2f>>& trajectories, const std::vector<int>& id_balls, const std::vector<cv::Point2f>& corners) {
    
    this->corners = corners;

    // Define the dimensions of the bird's-eye view
    cv::Size birdEyeViewSize(800, 400); // Adjust the size as needed

    // Load the table image
    cv::Mat tableImage = cv::imread("../res/table.png", cv::IMREAD_UNCHANGED);

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
    std::cout << " this " << this->corners << std::endl << " " << dstCorners;
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
        {0, cv::Scalar(0, 0, 255)}, // Red for ID 1
        {1, cv::Scalar(0, 255, 0)}, // Green for ID 2
        {2, cv::Scalar(255, 0, 0)}, // Blue for ID 3
        // Add more colors for additional IDs as needed
    };

    // Draw the balls and trajectories on the resized table image before overlaying
    for (size_t i = 0; i < birdEyeBallPositions.size(); ++i) {
        int id = id_balls[i];
        cv::Scalar color = colorMap[id % colorMap.size()]; // Assign color based on ID
        cv::Point2f pos = birdEyeBallPositions[i];
        cv::circle(resizedTableImage, pos, 5, color, -1);
    }

    // Draw the balls on the bird's-eye view
    /*for (const auto& pos : birdEyeBallPositions) {
        cv::circle(resizedTableImage, pos, 5, cv::Scalar(0, 0, 255), -1);
    }*/

    // Draw the trajectories on the bird's-eye view
    for (const auto& trajectory : birdEyeTrajectories) {
        for (size_t j = 1; j < trajectory.size(); ++j) {
            cv::line(resizedTableImage, trajectory[j - 1], trajectory[j], cv::Scalar(0, 255, 0), 2);
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
    cv::namedWindow("RISULTATO PROJECTION");
    cv::imshow("RISULTATO PROJECTION", frameWithOverlay);
    /* cv::waitKey(0);
    std::cout << "sono qui"; */
}