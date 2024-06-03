/*
Task 4
Expand Task 3 using the gaussian smoothing filter provided by OpenCV (you should find the
right function in the documentation). Show the results on the same images.
*/

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include "ImageFilters.h"

int main(int argc, char** argv){

    //load image and check
    if(argc < 2){
        std::cout << "ERROR: missing filepath. Please provide a filepath through cmd-line as argv[1]." << std::endl;
        return -1;
    }
    cv::Mat img = cv::imread(argv[1], cv::IMREAD_GRAYSCALE); //force load image with single-channel

    if(img.empty()){
        std::cout << "ERROR: invalid filepath. Please provide a valid one." << std::endl;
        return -1;
    }

    cv::namedWindow("original");
    cv::imshow("original", img);
    cv::Mat out;

    std::vector<int> sizes = {3,5,7,9,11,15};
    for (std::vector<int>::iterator i = sizes.begin(); i != sizes.end(); ++i){
        cv::GaussianBlur(img, out, cv::Size(*i,*i), 0);

        if(!out.empty()){
            std::string instance = "gaussFilter "+std::to_string(*i)+'x'+std::to_string(*i);
            cv::namedWindow(instance);
            cv::imshow(instance, out);
            cv::imwrite("Results/"+instance+".png", out);
            std::cout << "Press any key to continue..." << std::endl;
            cv::waitKey(0);
            cv::destroyWindow(instance);
        }
    }
    
    cv::destroyAllWindows();
    std::cout << "Results saved at 'Results/'." << std::endl;
    return 0;
}