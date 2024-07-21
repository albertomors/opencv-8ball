/*
    AUTHOR: Fresco Eleonora
    DATE: 2024-07-21
    FILE: metrics.h
    DESCRIPTION:Header of the class that handles metrics computation. 

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

#ifndef METRICS_INCLUDED
  #define METRICS_INCLUDED

  #include <opencv2/highgui.hpp>
  #include <opencv2/imgproc.hpp>
  #include <opencv2/opencv.hpp>
  #include <iostream>

  double compute_mAP(const cv::Mat& pred_bb, const cv::Mat& true_bb);
  double compute_IoU(const cv::Rect& r1, const cv::Rect& r2);
  std::vector<cv::Point2f> get_PR_table(const cv::Mat& pred_bb, const cv::Mat& true_bb, int pred_class);
  std::vector<cv::Point2f> refine_PR_table(std::vector<cv::Point2f>& points);
  
  double compute_mIoU(const std::vector<std::pair<cv::Mat, cv::Mat>>& videoSegMasks, int numClasses);
  double calculateIoU(const cv::Mat& groundTruth, const cv::Mat& prediction, int classId);

#endif