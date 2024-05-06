/*
Task 1
Write a program that loads the image provided (Garden) and shows it. It then converts the
image to grayscale (cv::cvtColor() function) and saves it as Garden_grayscale.
*/

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

int main(int argc, char** argv){

    //load image
    cv::Mat img = cv::imread("Images/Garden.jpg", cv::IMREAD_COLOR);
    if (img.empty()) {
        std::cout << "Image not found at specified path. Please provide a valid one recompiling task1.cpp \
        or place Garden.jpg in the correct position. [../Images/] respect to this file." << std::endl;
        return -1;
    }
    assert(img.type() == CV_8UC3);

    //shows it
    cv::namedWindow("original"); cv::imshow("original", img);
    std::cout << "Press any key to continue..." << std::endl;
    cv::waitKey(0);

    //converts it
    cv::Mat out;
    cv::cvtColor(img, out, cv::COLOR_BGR2GRAY);
    assert(out.type() == CV_8UC1);
    
    //shows converted
    cv::namedWindow("grayscale"); cv::imshow("grayscale", out);
    std::cout << "Press any key to continue..." << std::endl;
    cv::waitKey(0);

    cv::destroyAllWindows();

    std::string savepath = "Images/Garden_grayscale.jpg";
    cv::imwrite(savepath, out);
    std::cout << "Saved to '" << savepath << "'." << std::endl;

    return 0;
}