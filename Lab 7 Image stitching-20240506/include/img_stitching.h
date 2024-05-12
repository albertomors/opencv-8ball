//img_stitching.h

#ifndef IMG_STITCHING_H
#define IMG_STITCHING_H

#include <opencv2/core.hpp>

using namespace cv;

class ImgStitcher{
    public:
        std::vector<Mat> images, projected;
        double fov;

        std::vector<std::vector<KeyPoint>> keypoints;
        std::vector<Mat> descriptors;

        std::vector<std::vector<DMatch>> matches; //in position [i] the matches with [i+1]
        std::vector<Point2d> ds; //in position [i] dx,dy with [i+1]

        Mat final_img;

    public:
        ImgStitcher();
        
        bool load_images(const std::string& dir_name, const std::string& pattern, double fov);
        void project_images();

        void extract_features();
        void match_all();
        void display_match(int idx1);

        void compute_ds();
        void build_final_img();
};

#endif //IMG_STITCHING_H