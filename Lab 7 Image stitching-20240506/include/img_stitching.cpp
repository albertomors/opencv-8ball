//img_stitching.cpp

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>

#include <opencv2/core/utils/filesystem.hpp>
#include <filesystem>

#include "img_stitching.h"
#include "panoramic_utils.h"

using namespace cv;
using namespace cv::xfeatures2d;

ImgStitcher::ImgStitcher() {}

bool ImgStitcher::load_images(const std::string& dir_name, const std::string& pattern, double fov) {
    std::string root_path = std::filesystem::current_path().string() + "/../Images/" + dir_name;
    this->fov = fov;
    
    //array of paths of every image
    std::vector<std::string> imgs_paths;
    try{
        cv::utils::fs::glob(root_path, pattern, imgs_paths);
    }
    catch(...){
        std::cerr << "No images found in directory: " << root_path << std::endl;
        return false;
    }
    
    //iteratively load
    for (const auto& path : imgs_paths) {
        Mat img = imread(path);
        if (img.empty()) {
            std::cerr << "Failed to load image: " << path << std::endl;
            return false;
        }
        this->images.push_back(img);
    }

    std::cout << "Succesfully loaded " << images.size() << " images" << std::endl;
    return true;
}

void ImgStitcher::project_images() {
    for (const auto& img : this->images){
        this->projected.push_back(cylindricalProj(img,this->fov));
    }
    std::cout << "Succesfully projected " << this->projected.size() << " images." << std::endl;
}

void ImgStitcher::extract_features() {
    std::cout << "Extracting features..." << std::endl;
    Ptr<SIFT> detector = SIFT::create();

    for (const auto& img : this->projected){
        std::vector<KeyPoint> keypoints_single;
        Mat descriptors_single;
        detector->detectAndCompute(img, cv::noArray(), keypoints_single, descriptors_single);
        this->keypoints.push_back(keypoints_single);
        this->descriptors.push_back(descriptors_single);
        std::cout << "Extracted " << keypoints_single.size() << " keypoints with a " << descriptors_single.size() << " descriptor." << std::endl;
    }

    CV_Assert(this->keypoints.size() == this->descriptors.size());
    std::cout << "Succesfully extracted features on " << this->keypoints.size() << " images." << std::endl;
}

void ImgStitcher::match_all(){
    std::cout << "Matching features..." << std::endl;
    Ptr<BFMatcher> matcher = BFMatcher::create();

    std::vector<DMatch> matches_single;
    for(size_t i=0; i<this->descriptors.size()-1; i++){
        matcher->match(this->descriptors[i], this->descriptors[i+1], matches_single);
        
        //find minimum distance
        double min_dist = matches_single[0].distance;
        for (size_t j=1; j<matches_single.size(); j++) {
            if (matches_single[j].distance < min_dist) {
                min_dist = matches_single[j].distance;
            }
        }
        std::cout << "Found " << matches_single.size() << " matches on imgs " << i << "," << i+1 << ". " \
                  << "Minimum distance found: " << min_dist << ". ";

        //trash bad matches
        std::vector<DMatch> good_matches_single;
        double ratio_treshold = 3.0f;
        for (size_t j=0; j<matches_single.size(); j++) {
            if (matches_single[j].distance < ratio_treshold * min_dist) {
                good_matches_single.push_back(matches_single[j]);
            }
        }

        std::cout << "From " << matches_single.size() << " kept " << good_matches_single.size() << " good matches." << std::endl;
        this->matches.push_back(good_matches_single);
    }
    CV_Assert(this->keypoints.size()-1 == this->matches.size());
    std::cout << "Succesfully found " << this->matches.size() << " matches." << std::endl;
}

void ImgStitcher::compute_ds(){
    std::cout << "Computing translation..." << std::endl;

    //iterate on images
    for(size_t i=0; i<this->matches.size(); i++){
    
        std::vector<Point2d> p1, p2;
        //iterate on [m] matches saved in [i] found on images [i] and [i+1]
        for (size_t m = 0; m < this->matches[i].size(); m++){
            p1.push_back(this->keypoints[i][this->matches[i][m].queryIdx].pt); //pick corresponding keypoint on [i]
            p2.push_back(this->keypoints[i+1][this->matches[i][m].trainIdx].pt); //pick corresponding keypoint on [i+1]
        }

        Mat mask;
        Mat homography = findHomography(p1, p2, RANSAC, 3.0, mask); //only purpose of homography is to keep mask of inliers
        int num = 0;
        Point2d ds_single = {0.0, 0.0};
        for (size_t j=0; j<mask.rows; j++){
            if (mask.at<uchar>(j)){
                num++; //num of inliers
                ds_single += p2[j] - p1[j];
            }
        }
        ds_single = -ds_single/num;
        std::cout << "From " << this->matches[i].size() << " kept " << num << " inliers for imgs " << i << "," << i+1 <<". Average ds: " << ds_single << std::endl;
        this->ds.push_back(ds_single); //push - to get actual translation
    }
    std::cout << "Succesfully processed " << this->ds.size() << " (dx,dy)." << std::endl;
}

void ImgStitcher::build_final_img(){
    int w0, h0;
    double x, y, xmin, xmax, ymin, ymax;
    w0 = this->projected[0].size().width;
    h0 = this->projected[0].size().height;
    x = 0;
    y = 0;
    xmin = 0; //works also for opposite rotation set of images
    xmax = 0;
    ymin = 0;
    ymax = 0;
    //iterate on displacement to find extremas to define bigger final image
    for (int i=0; i<this->ds.size(); i++){
        x += ds[i].x;
        y += ds[i].y;
        xmin = std::min(xmin, x);
        xmax = std::max(xmax, x);
        ymin = std::min(ymin, y);
        ymax = std::max(ymax, y);
    }
    //now i have the maximum excursion from first image in both direction
    int final_w = w0 - xmin + xmax;
    int final_h = h0 - ymin + ymax;

    this->final_img = Mat(final_h, final_w, CV_8UC1, Scalar(0,0,0));
    std::cout << "Size: " << this->projected[0].size() << " to " << this->final_img.size() << std::endl;
    
    x = 0;
    y = -ymin;
    for (int i=0; i<this->projected.size(); i++){
        Rect rect(x,y,this->projected[i].size().width,this->projected[i].size().height);
        this->projected[i].copyTo(this->final_img(rect));
        x += ds[i].x;
        y += ds[i].y;
    }
}