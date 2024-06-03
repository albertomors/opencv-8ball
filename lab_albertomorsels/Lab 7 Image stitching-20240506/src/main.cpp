//main.cpp

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include "../include/img_stitching.h"

int main(int argc, char** argv) {
    if (argc < 2){
        std::cerr << "Pass folder name to load. Example: >>./main dolomites" << std::endl;
        return -1;
    }
    double fov = (argv[1] == static_cast<std::string>("dolomites")) ? 27 : 33;
    ImgStitcher processor;
    
    if(!processor.load_images(argv[1], "i*", fov)){
        return -1;
    }
    processor.project_images();
    
    //example
    namedWindow("image"); imshow("image",processor.images[0]);
    namedWindow("projection"); imshow("projection",processor.projected[0]);
    waitKey(0); destroyAllWindows();

    processor.extract_features();
    processor.match_all();

    //example
    Mat img_matches;
    drawMatches(processor.projected[0], processor.keypoints[0], processor.projected[1], processor.keypoints[1], processor.matches[0], \
                img_matches, Scalar::all(-1), Scalar::all(-1), std::vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
    namedWindow("matches"); imshow("matches", img_matches);
    waitKey(0); destroyWindow("matches");

    processor.compute_ds();
    processor.build_final_img();

    namedWindow("final"); imshow("final", processor.final_img);
    waitKey(0); destroyWindow("final");

    return 0;
}