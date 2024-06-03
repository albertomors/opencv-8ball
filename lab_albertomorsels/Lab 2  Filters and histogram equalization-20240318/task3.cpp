/*
Task 3
Expand Task 2 implementing the median filter in a dedicated function in the same
header/source files used for task 2. Test the filter on the same images.
*/

#include <opencv2/highgui.hpp>
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

    //apply medFilter. Try first with invalid kernel_size, and right after with valid ones.
    std::vector<int> sizes = {1,3,5,7,9,11,15};
    for (std::vector<int>::iterator i = sizes.begin(); i != sizes.end(); ++i){

        out = medFilter(img, *i);
        //shows it only if the application was succesful
        if(!out.empty()){
            std::string instance = "medFilter "+std::to_string(*i)+'x'+std::to_string(*i);
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