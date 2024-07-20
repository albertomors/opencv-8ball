/*
AUTHOR: Girardello Sofia 
*/

#include "trajectoryTracking.h"
 #include <opencv2/tracking.hpp>

// Constructor to initialize the frame
trajectoryTracker::trajectoryTracker() {
}


void trajectoryTracker::initializeTrackers(const cv::Mat& frame, const std::vector<cv::Rect>& initial_bboxes){

    cv::TrackerCSRT::Params csrtParams;
    csrtParams.use_hog = true;               // Use HOG features
    csrtParams.use_color_names = true;       // Use Color Names features
    csrtParams.use_gray = true;              // Use Gray features
    csrtParams.use_rgb = true;               // Use RGB features
    csrtParams.use_channel_weights = true;   // Use Channel Weights
    csrtParams.use_segmentation = true;      // Use Segmentation
    csrtParams.window_function = "hann";     // Window function
    csrtParams.kaiser_alpha = 4.75;          // Kaiser window parameter
    csrtParams.cheb_attenuation = 45;        // Chebyshev window parameter
    csrtParams.template_size = 400;          // Template size
    csrtParams.gsl_sigma = 2.0;              // Gaussian window parameter
    csrtParams.hog_orientations = 9;         // HOG orientations
    csrtParams.num_hog_channels_used = 18;   // Number of HOG channels
    csrtParams.filter_lr = 0.03;             // Learning rate for the filter
    csrtParams.weights_lr = 0.03;            // Learning rate for the weights
    csrtParams.admm_iterations = 7;          // Number of ADMM iterations
    csrtParams.number_of_scales = 33;        // Number of scales
    csrtParams.scale_sigma_factor = 0.25;    // Scale sigma factor
    csrtParams.scale_model_max_area = 512;   // Scale model max area
    csrtParams.scale_lr = 0.001;             // Scale learning rate
    csrtParams.scale_step = 1.01;            // Scale step
    csrtParams.psr_threshold = 0.05;         // PSR threshold


    for (const auto& bbox : initial_bboxes) {
            cv::Ptr<cv::Tracker> tracker = cv::TrackerCSRT::create(csrtParams);
            tracker->init(frame, bbox);
            this->trackers.push_back(tracker);
            this->ballTrajectories.push_back(std::vector<cv::Point2f>());
    }

}


   void trajectoryTracker::updateTrackers(const cv::Mat& frame) {

        // Clear previous centers and trajectories
        this->centers.clear();
        this->trajectories.clear();

        // Update all trackers
        for (size_t i = 0; i < this->trackers.size(); ++i) {
            cv::Rect bbox;
            bool ok = this->trackers[i]->update(frame, bbox);
            if (ok) {
                // Draw bounding box
                cv::rectangle(frame, bbox, cv::Scalar(255, 0, 0), 2, 1);

                cv::Point2f center(bbox.x + bbox.width / 2, bbox.y + bbox.height / 2);
                this->ballTrajectories[i].push_back(center);

                // Draw the trajectory
                for (size_t j = 1; j < this->ballTrajectories[i].size(); ++j) {
                    cv::line(frame, this->ballTrajectories[i][j - 1], this->ballTrajectories[i][j], cv::Scalar(0, 255, 0), 2);
                }

                // Draw the center
                cv::circle(frame, center, 5, cv::Scalar(0, 255, 0), -1);

                // Store the center and trajectory
                this->centers.push_back(center);
                this->trajectories.push_back(this->ballTrajectories[i]);

            } else {
                std::cout << "Tracker " << i << " lost the object." << std::endl;
            }
        }

        cv::imshow("Ball Tracking", frame);
        //cv::waitKey(1);
    }

