/*
Task 2
Expand Task 1 implementing a max filter and a min filter in two dedicated functions
(implemented by you in separate header/source files named ImageFilters.h and
ImageFilters.cpp) that manipulate the pixels directly. Such functions consider square kernels
whose size is provided as an argument. The size should be an odd number—if it is even, the
function does not process the image and prints an error message.
Apply the filters using several kernel sizes and find the option that:
● best removes the noise from the Lena_corrupted image (compare with the original
version);
● best removes the noise from the Astronaut_corrupted image (compare with the
original version);
● best removes the electric cables in the background from the Garden_grayscale
without corrupting the image too much.
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

    //apply maxFilter. Try first with invalid kernel_size, and right after with valid ones.
    std::vector<int> sizes = {2,1,3,5,7,11};
    for (std::vector<int>::iterator i = sizes.begin(); i != sizes.end(); ++i){

        out = maxFilter(img, *i);
        //shows it only if the application was succesful
        if(!out.empty()){
            std::string instance = "maxFilter "+std::to_string(*i)+'x'+std::to_string(*i);
            cv::namedWindow(instance);
            cv::imshow(instance, out);
            cv::imwrite("Results/"+instance+".png", out);
            std::cout << "Press any key to continue..." << std::endl;
            cv::waitKey(0);
            cv::destroyWindow(instance);
        }
    }

    //apply minFilter
    for (std::vector<int>::iterator i = sizes.begin(); i != sizes.end(); ++i){

        out = minFilter(img, *i);
        //shows it only if the application was succesful
        if(!out.empty()){
            std::string instance = "minFilter "+std::to_string(*i)+'x'+std::to_string(*i);
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