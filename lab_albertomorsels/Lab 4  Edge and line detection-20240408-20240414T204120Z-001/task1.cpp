
/*
Task 1
Write a program that loads the image provided (street_scene.png), shows it and evaluates
the Canny image. To verify the effect on the final result, add one or more trackbar(s)1 to
control the parameters of the Canny edge detector. Move the trackbars and check how
changing each parameter has an influence on the resulting image. Please note: the Canny
image shall be refreshed every time a trackbar is modified.
*/

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

int t1, t2; //tresholds 1 and 2
int k1, k2; //sobel kernel size, gaussian

void onTrackbar(int pos, void* userdata){

    cv::Mat* img = static_cast<cv::Mat*>(userdata);

    cv::Mat imgray, edges;
    cv::cvtColor(*img, imgray, cv::COLOR_BGR2GRAY);  //convert to grayscale

    k1 = (k1%2!=0) ? k1 : std::max(3, k1-1);
    k2 = (k2%2!=0) ? k2 : std::max(3, k2-1);
    std::cout << "GAUSS: " << k1 << " SOBEL: " << k2 << std::endl;

    cv::blur(imgray, imgray, cv::Size(k1,k1) );      //blur
    cv::Canny(imgray, edges, t1, t2, k2);            //canny
    cv::imshow("processed", edges);
    }

int main(int argc, char** argv){

    cv::Mat img = cv::imread("street_scene.png", cv::IMREAD_ANYCOLOR);
    if (img.empty()) {
        std::cout << "ERROR: invalid filepath. Place 'street_scene.png' in the same folder." << std::endl;
        return -1; }

    cv::namedWindow("street_scene.png");
    cv::imshow("street_scene.png", img);

    //params to pass to callbacks
    cv::namedWindow("processed");
    k1 = 3;
    k2 = 3;

    //create trackbars
    cv::createTrackbar("treshold 1", "processed", &t1, 255, onTrackbar, static_cast<void*>(&img));
    cv::createTrackbar("treshold 2", "processed", &t2, 255, onTrackbar, static_cast<void*>(&img));
    cv::createTrackbar("GAUSS kers", "processed", &k1, 30,  onTrackbar, static_cast<void*>(&img));
    cv::setTrackbarMin("GAUSS kers", "processed", 3);
    cv::createTrackbar("SOBEL kers", "processed", &k2, 7,  onTrackbar, static_cast<void*>(&img));
    cv::setTrackbarMin("SOBEL kers", "processed", 3);

    cv::waitKey(0);
    cv::destroyAllWindows();

    return 0;
}