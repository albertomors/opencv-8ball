/*
Task 5
Write a program that creates two images of size 256x256, one channel, 8-bit depth unsigned
char. Write the pixels using the Mat::at() function in order to create:
● a vertical gradient in the first image;
● a horizontal gradient in the second image.
Show the images on screen.
*/

#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>
#include <iostream>

int main(int argc, char** argv)
{   
    cv::Mat img1 = cv::Mat(256,256,CV_8UC1);
    cv::Mat img2 = img1.clone();

    for(int x=0; x<img1.cols; ++x){
        for(int y=0; y<img1.rows; ++y){
            img1.at<uchar>(y,x) = y;
            img2.at<uchar>(y,x) = x;
        }
    }

    cv::imshow("vertical",img1);
    cv::imshow("horizontal",img2);
    cv::waitKey(0);

    /*
    Expand the software to create and visualize two other images,
    size 300x300, same features as above, with:
    ● a chessboard with squares of size 20 pixels;
    ● a chessboard with squares of size 50 pixels.
    */

    img1 = cv::Mat(300,300,CV_8UC1);
    img2 = img1.clone();

    for(int x=0; x<img1.cols; ++x){
        for(int y=0; y<img1.rows; ++y){
            img1.at<uchar>(y,x) = ((x/20 + y/20)%2) ? 255 : 0;
            img2.at<uchar>(y,x) = ((x/50 + y/50)%2) ? 255 : 0;
        }
    }

    cv::imshow("20",img1);
    cv::imshow("50",img2);
    cv::waitKey(0);

    return 0;
}