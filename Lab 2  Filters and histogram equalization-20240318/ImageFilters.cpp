//ImageFilters.cpp

#include <opencv2/core.hpp>
#include <iostream>
#include "ImageFilters.h"

//------------------------------------------------------------------------

cv::Mat maxFilter(const cv::Mat& src, int kernel_size) {

    //check correctness
    assert(src.type() == CV_8UC1);  //able to process just single-channels
    if(kernel_size%2 == 0){
        std::cout << "ERROR: invalid kernel_size. Please provide a valid (odd) kernel_size and retry." << std::endl;
        return cv::Mat();           //return an empty result
    }
    cv::Mat res = src.clone();      //deep-copy of the source

    int off = (kernel_size-1) / 2;      //kernel's offset from center to boundaries
    int y1,y2,x1,x2;                    //actual boundaries (to handle corner scenarios)
    uchar max_val;                      //taken outside from loop to optimize speed

    //process src image
    for (int y=0; y<src.rows; ++y){
        for (int x=0; x<src.cols; ++x){
            
            y1 = std::max(0,          y-off);
            y2 = std::min(src.rows-1, y+off);
            x1 = std::max(0,          x-off);
            x2 = std::min(src.cols-1, x+off);

            // iterate over the neighborhood
            max_val = 0; //last-found max - initialize with worst possible value (0);
            for (int m=y1; m<=y2; ++m)
                for (int n=x1; n<=x2; ++n)
                    max_val = std::max(src.at<uchar>(m,n), max_val); //update-rule

            //write the pixel
            res.at<uchar>(y,x) = max_val;
        }
    }

    return res;
}

//----------------------------------------------------------------------

cv::Mat minFilter(const cv::Mat& src, int kernel_size) {

    //check correctness
    assert(src.type() == CV_8UC1);  //able to process just single-channels
    if(kernel_size%2 == 0){
        std::cout << "ERROR: invalid kernel_size. Please provide a valid (odd) kernel_size and retry." << std::endl;
        return cv::Mat();           //return an empty result
    }
    cv::Mat res = src.clone();      //deep-copy of the source

    int off = (kernel_size-1) / 2;      //kernel's offset from center to boundaries
    int y1,y2,x1,x2;                    //actual boundaries (to handle corner scenarios)
    uchar min_val;                      //taken outside from loop to optimize speed

    //process src image
    for (int y=0; y<src.rows; ++y){
        for (int x=0; x<src.cols; ++x){
            
            y1 = std::max(0,          y-off);
            y2 = std::min(src.rows-1, y+off);
            x1 = std::max(0,          x-off);
            x2 = std::min(src.cols-1, x+off);

            // iterate over the neighborhood
            min_val = 255; //last-found min - initialize with worst possible value (255);
            for (int m=y1; m<=y2; ++m)
                for (int n=x1; n<=x2; ++n)
                    min_val = std::min(src.at<uchar>(m,n), min_val); //update-rule

            //write the pixel
            res.at<uchar>(y,x) = min_val;
        }
    }

    return res;
}

//----------------------------------------------------------------------

cv::Mat medFilter(const cv::Mat& src, int kernel_size) {

    //check correctness
    assert(src.type() == CV_8UC1);  //able to process just single-channels
    if(kernel_size%2 == 0){
        std::cout << "ERROR: invalid kernel_size. Please provide a valid (odd) kernel_size and retry." << std::endl;
        return cv::Mat();           //return an empty result
    }
    cv::Mat res = src.clone();      //deep-copy of the source

    int off = (kernel_size-1) / 2;      //kernel's offset from center to boundaries
    int y1,y2,x1,x2;                    //actual boundaries (to handle corner scenarios)
    std::vector<uchar> values;
    values.resize(kernel_size*kernel_size); //pre-allocate space avoiding resizing at every iteration
    int i;

    //process src image
    for (int y=0; y<src.rows; ++y){
        for (int x=0; x<src.cols; ++x){
            
            y1 = std::max(0,          y-off);
            y2 = std::min(src.rows-1, y+off);
            x1 = std::max(0,          x-off);
            x2 = std::min(src.cols-1, x+off);

            i = 0; //reset-cell to write from the previous iteration
            // iterate over the neighborhood
            for (int m=y1; m<=y2; ++m){
                for (int n=x1; n<=x2; ++n){
                    values[i++] = src.at<uchar>(m,n); //re-fill the array
                }
            }

            std::sort(values.begin(), values.end());                //sort the values
            res.at<uchar>(y,x) = values[(values.size()-1) / 2];     //pick the median
        }
    }

    return res;
}