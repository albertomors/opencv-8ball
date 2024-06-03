
/*
Task 2
You now need to detect the white markings on the road. How could you tackle this problem
without using the Hough transform? Some suggestions:
● consider edge orientation;
● consider colors close to edge points.
*/

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

int t1, t2;

void onTrackbar(int pos, void* userdata){

    cv::Mat* img = static_cast<cv::Mat*>(userdata);

    //treshold
    cv::Mat imgray, tresh;
    cv::cvtColor(*img, imgray, cv::COLOR_BGR2GRAY);
    cv::threshold(imgray, tresh, t2, 255, cv::THRESH_BINARY);
    cv::imshow("tresh", tresh);

    //compute gradient
    cv::Mat gradX, gradY, mag, angle;
    cv::Scharr(tresh, gradX, CV_32F, 1, 0);
    cv::Scharr(tresh, gradY, CV_32F, 0, 1);
    cv::cartToPolar(gradX, gradY, mag, angle, true); //deg

    cv::normalize(mag, mag, 0, 255, cv::NORM_MINMAX);
    mag.convertTo(mag, CV_8U);
    cv::namedWindow("mag"); cv::imshow("mag", mag);

    cv::Mat filt;
    cv::Mat mask = ~((angle >= (-t1) & angle <= t1) | (angle >= (180-t1) & angle <= (180+t1))); //cuts the vertical
    mask &= ~((angle >= (90-t1) & angle <= (90+t1)) | (angle >= (270-t1) & angle <= (270+t1))); //cuts the horiz
    mag.copyTo(filt, mask);
    cv::namedWindow("filt"); cv::imshow("filt", filt);
}

int main(int argc, char** argv){

    cv::Mat img = cv::imread("street_scene.png", cv::IMREAD_ANYCOLOR);
    if (img.empty()) {
        std::cout << "ERROR: invalid filepath. Place 'street_scene.png' in the same folder." << std::endl;
        return -1; }

    cv::namedWindow("tresh");
    cv::createTrackbar("alpha tol", "tresh", &t1, 45, onTrackbar, static_cast<void*>(&img));
    cv::createTrackbar("treshold", "tresh", &t2, 255, onTrackbar, static_cast<void*>(&img));

    cv::waitKey(0);
    cv::destroyAllWindows();

    return 0;
}