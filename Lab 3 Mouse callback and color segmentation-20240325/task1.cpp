/*
Task 1
Write a program that loads the image provided (robocup.jpg) and shows it.
*/

#include <opencv2/highgui.hpp>
#include <iostream>

int main(int argc, char** argv){

    //load image
    cv::Mat img = cv::imread("robocup.jpg", cv::IMREAD_ANYCOLOR);
    if (img.empty()) {
        std::cout << "ERROR: invalid filepath. Place 'robocup.jpg' in the same folder." << std::endl;
        return -1;
    }

    cv::namedWindow("robocup.jpg");
    cv::imshow("robocup.jpg", img);
    cv::waitKey(0);
    cv::destroyAllWindows();

    return 0;
}
