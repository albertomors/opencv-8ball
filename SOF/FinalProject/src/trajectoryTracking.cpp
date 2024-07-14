/*
AUTHOR: Girardello Sofia 
*/

#include "trajectoryTracking.h"
 #include <opencv2/tracking.hpp>

// Constructor to initialize the frame
trajectoryTracker::trajectoryTracker() {
}

/*
void trajectoryTracker::initializeTrackers(const cv::Mat& frame, const std::vector<cv::Point2f>& centers) {

    std::vector<cv::Rect2d> ballRects; 

    for (size_t i = 0; i < centers.size(); ++i) {

        cv::Ptr<cv::Tracker> tracker = cv::TrackerKCF::create(); 

        cv::Rect2d rect(centers[i].x - 10, centers[i].y - 10, 20, 20);
        ballRects.push_back(rect);

        tracker->init(frame, rect);
        this->trackers.push_back(tracker);

        // Initialize trajectory with the first position
        std::vector<cv::Point2f> trajectory;
        trajectory.push_back(centers[i]);
        this->ballTrajectories.push_back(trajectory);
    }
    // Print trajectories for debugging
        for (const auto& traj : ballTrajectories) {
            for (const auto& point : traj) {
                std::cout << point << " ";
            }
            std::cout << std::endl;
        }
}


void trajectoryTracker::updateTrackers(const cv::Mat& frame, const std::vector<cv::Point2f>& centers) {
    for (size_t i = 0; i < trackers.size(); ++i) {
        // Convert cv::Point2f centers[i] to cv::Rect
        cv::Rect bbox(static_cast<int>(centers[i].x - 10), 
                      static_cast<int>(centers[i].y - 10), 
                      20, 20);

        bool ok = trackers[i]->update(frame, bbox);
        if (ok) {
            // Update the ball trajectory with the new position
            ballTrajectories[i].push_back(cv::Point2f(bbox.x + bbox.width / 2, bbox.y + bbox.height / 2));

            // Optionally draw the current position on the frame
            cv::circle(frame, cv::Point(bbox.x + bbox.width / 2, bbox.y + bbox.height / 2), 5, cv::Scalar(255, 255, 0), -1);
        } else {
            // Handle tracker failure (optional: reinitialize or remove tracker)
            std::cout << "Tracker " << i << " failed. Removing tracker." << std::endl;
            trackers.erase(trackers.begin() + i);
            ballTrajectories.erase(ballTrajectories.begin() + i);
            --i; // Adjust index after erasing
        }
    }
    cv::imshow("Trajectories", frame);
}


void trajectoryTracker::drawTrajectories(cv::Mat& frame) {
        for (const auto& trajectory : ballTrajectories) {
            for (size_t i = 1; i < trajectory.size(); ++i) {
                cv::line(frame, trajectory[i - 1], trajectory[i], cv::Scalar(0, 255, 0), 2);  // Green trajectory lines
            }
        }
        cv::imshow("Trajectories", frame);
    }

*/

/*
void trajectoryTracker::initializeTrackers(const cv::Mat& frame, const std::vector<cv::Rect>& initial_bboxes) {
    cv::MultiTracker multiTracker;
    multiTracker = cv::MultiTracker.create();

    for (const auto& bbox : initial_bboxes) {
        multiTracker->add(cv::TrackerTLD::create(), frame, bbox);
        std::vector<cv::Point2f> trajectory;
        trajectory.push_back(cv::Point2f(bbox.x + bbox.width / 2.0f, bbox.y + bbox.height / 2.0f));
        ballTrajectories.push_back(trajectory);
    }
}
*/

