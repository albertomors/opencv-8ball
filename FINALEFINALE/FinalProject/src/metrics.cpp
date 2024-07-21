/*
    AUTHOR: Morselli Alberto
    DATE: 2024-07-21
    FILE: metrics.cpp
    DESCRIPTION: Implements functions for computing various performance metrics related to object detection and tracking. 

    FUNCTIONS:
    - double compute_IoU(...): Computes the Intersection over Union (IoU) between two bounding boxes.
    - std::vector<cv::Point2f> get_PR_table(...): Generates a Precision-Recall (PR) table for a specific class based on predicted and ground truth bounding boxes.
    - std::vector<cv::Point2f> refine_PR_table(...): Refines the PR table by removing coincident points and flattening segments to the next maximum.
    - double compute_mAP(...): Computes the mean Average Precision (mAP) over all classes using the PR table.
    - double compute_IoU_px(...): Calculates the IoU for a specific class at the pixel level.
    - double compute_mIoU(...): Computes the mean IoU (mIoU) between ground truth and predicted segmentation masks over a video sequence.

    NOTES:
    - IoU is computed by finding the intersection and union of two bounding boxes.
    - PR tables are generated to evaluate precision and recall at different thresholds.
    - mAP is computed by averaging the AP values over all classes.
    - mIoU is computed over the entire video sequence by comparing segmentation masks for the first and last frames.

    USAGE:
    - These metrics are used to evaluate the performance of object detection and tracking algorithms by comparing predicted results with ground truth.
*/

#include "metrics.h"

//r1 = predicted_bb, r2=groundtruth_bb or viceversa
double compute_IoU(const cv::Rect& r1, const cv::Rect& r2){
    cv::Rect I_rect = r1 & r2;
    int I_area = I_rect.area();
    int U_area = r1.area() + r2.area() - I_area;
    double IoU = static_cast<float>(I_area)/static_cast<float>(U_area);
    return IoU;
}

/*
both pred can have less rows than true if we have missed predictions, more rows if we have FP
both have 5 <uint16_t> items per row = x y w h class
*/
std::vector<cv::Point2f> get_PR_table(const cv::Mat& pred_bb, const cv::Mat& true_bb, int pred_class){
    // Initialize cumulative indexes
    int TP = 0;
    int FP = 0;
    float P, R;
    std::vector<cv::Point2f> PR_points;

    // Pre-iteration over groundtruth to get total GT
    int totalGT = 0;
    for(int i=0; i<true_bb.rows; ++i)
        totalGT += (true_bb.at<uint16_t>(i,4) == pred_class);

    // Handle the scenario where no matching GT (true_bb) is present for the class
    // All preds will be FP => TP=0 - avoid all checks
    if(totalGT == 0){
        cv::Point2f point(1.0,0.0);
        PR_points.push_back(point); //append point to array
        std::cout << "No GT are present for class " << pred_class << std::endl;
        return PR_points;
    }

    // Iterate over predictions
    uint16_t x,y,w,h;
    for(int i=0; i<pred_bb.rows; ++i){
        // But look only at the ones belonging to the class we're analyzing
        if(pred_bb.at<uint16_t>(i,4) == pred_class){ 
            bool isTP = false;

            x = pred_bb.at<uint16_t>(i,0);
            y = pred_bb.at<uint16_t>(i,1);
            w = pred_bb.at<uint16_t>(i,2);
            h = pred_bb.at<uint16_t>(i,3);
            cv::Rect pred_rect(x,y,w,h);
            
            // Iterate all over groudtruths
            for(int j=0; j<true_bb.rows; ++j){
                // But look only at the ones belonging to the same class
                if(true_bb.at<uint16_t>(j,4) == pred_class){ 
                    x = true_bb.at<uint16_t>(j,0);
                    y = true_bb.at<uint16_t>(j,1);
                    w = true_bb.at<uint16_t>(j,2);
                    h = true_bb.at<uint16_t>(j,3);
                    cv::Rect true_rect(x,y,w,h);

                    float IoU = compute_IoU(pred_rect, true_rect);
                    //--Debug  std::cout << i << "," << j << "] " << pred_rect << "vs" << true_rect << " = " << IoU << std::endl;
                    
                    if( IoU >= 0.5){
                        // Found the matching => true positive - stop searching
                        isTP = true;
                        break;
                    }
                    // Otherwise continue looking for the potential matching
                }
            }

            if(isTP){TP++;}
            else{FP++;}

            // Compute P,R indexes
            P = static_cast<float>(TP)/static_cast<float>(TP+FP);
            R = static_cast<float>(TP)/static_cast<float>(totalGT); //true_bb.rows = total groundtruth
            float R_quantized = std::round(R * 10.0f) / 10.0f; //round the R values to +-0.1 for 11-point interpolation

            //--Debug  std::cout << TP << ":" << FP << " | PR=" << P << ":" << R << "-->" << R_quantized << std::endl;
            
            cv::Point2f point(R_quantized,P); // R=x, P=y
            PR_points.push_back(point); // Append point to array
        }
    }

    // Handle the scenario where no prediction bb is present => all pred are FN or TN
    if(PR_points.size() == 0){
        // At this point if we have GTs but no pred = FN (bad)
        // Otherwise if no pred but also no GT => TN (good)
        cv::Point2f point = (totalGT == 0) ? cv::Point2f(1.0,1.0) : cv::Point2f(1.0,0.0);
        PR_points.push_back(point); //append point to array
        std::cout << "No predictions are present for class " << pred_class << std::endl;
        return PR_points;
    }

    return PR_points;
}

// Remove coincident points (deleting the lowest) also flattening oblique segments to the next max.
std::vector<cv::Point2f> refine_PR_table(std::vector<cv::Point2f>& points){

    // Keep non overlapping x-points
    std::vector<cv::Point2f> unique_points;
    float last_seen = points[0].x;
    unique_points.push_back(points[0]);

    for(int i=1; i<points.size(); ++i){
        if(points[i].x == last_seen){ 
            // Since only v[i+1] <= v[i] can happen | keep the max == [i], remove [i+1]
            continue; 
        }
        // Otherwise push it and update last seen
        unique_points.push_back(points[i]);
        last_seen = points[i].x;
    }

    // Now flat the y-values from right to left
    for(int i=unique_points.size()-1; i>0; --i){
        unique_points[i-1].y = std::max(unique_points[i-1].y, unique_points[i].y);
    }

    return unique_points;
}

double compute_mAP(const cv::Mat& pred_bb, const cv::Mat& true_bb){
    double mAP = 0.0;
    const int num_classes = 4;

    // Iterate over all classes
    for(int i=1; i<=num_classes; ++i){
        std::vector<cv::Point2f> PR_points = get_PR_table(pred_bb, true_bb, i);
        PR_points = refine_PR_table(PR_points);

        //AP = 1/11 * first + 1/11 * sum(points)
        double AP = 1.0/11 * PR_points[0].y + 10.0/11 * PR_points[0].x * PR_points[0].y;
        for(int j=1; j<PR_points.size(); j++){
            AP += 10.0/11 * (PR_points[j].x - PR_points[j-1].x) * PR_points[j].y; //dx*y
        }
        /*
        the expression is equivalent to summing 11-values weighted by 1/11 [first sample is inclusive]
        1/11 * 2 * y = 1/11 * 10 * 0.2 * y = 10/11 * 0.2 * y
        */

        //std::cout << "AP for class " << i << " = " << AP << std::endl; //--debug

        mAP += AP;
    }

    mAP/=num_classes;
    return mAP;
}


// Function to calculate IoU for a single class (from 0 to 5)
double compute_IoU_px(const cv::Mat& groundTruth, const cv::Mat& prediction, int class_id) {
    cv::Mat intersection, union_;
    CV_Assert(groundTruth.channels() == 1 && prediction.channels() == 1);

    // Create binary masks for the class
    cv::Mat gtBinary = (groundTruth == class_id);
    cv::Mat predBinary = (prediction == class_id);

    // Calculate intersection and union
    cv::bitwise_and(gtBinary, predBinary, intersection);
    cv::bitwise_or(gtBinary, predBinary, union_);

    int intersectionArea = cv::countNonZero(intersection);
    int unionArea = cv::countNonZero(union_);

    // To avoid division by 0
    if (unionArea == 0) {
        return 0.0;
    }
    // Else return IoU
    double IoU = static_cast<double>(intersectionArea) / unionArea;
    return IoU;
}

// Function to compute the average mIoU for the considered video
double compute_mIoU(const std::vector<std::pair<cv::Mat, cv::Mat>>& seg_masks, int num_classes) {
    // Ensure we have exactly two pairs of masks (first and last?)
    if (seg_masks.size() != 2) {
        std::cerr << "Exactly two mask pairs are required for mIoU computation." << std::endl;
        return 0.0;
    }
    if (num_classes == 0){
        return 0.0;
    }

    const auto& ffirst_pair = seg_masks[0];
    const auto& flast_pair = seg_masks[1];

    const cv::Mat& gt_ffirst = ffirst_pair.first;
    const cv::Mat& pred_ffirst = ffirst_pair.second;
    const cv::Mat& gt_flast = flast_pair.first;
    const cv::Mat& pred_flast = flast_pair.second;
    
    double total_miou = 0.0;

    // Calculate IoU for all classes
    for (int class_id = 0; class_id < num_classes; ++class_id) {
        double ffirst_iou = compute_IoU_px(gt_ffirst, pred_ffirst, class_id);
        double flast_iou = compute_IoU_px(gt_flast, pred_flast, class_id);
        double miou = (ffirst_iou + flast_iou) / 2.0;
        total_miou += miou;

        //std::cout << "IoU for class " << class_id << " = " << miou << std::endl; //--debug
    }

    // Return mIoU
    return total_miou / num_classes;
}