
/*
Task 4
Detect the road sign using the Hough circular transform - function cv::HoughCircles().
*/

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int dist, minradius, maxradius, p1, p2;

void onTrackbar(int pos, void* userdata){

    cv::Mat* img_ptr = static_cast<cv::Mat*>(userdata);
    cv::Mat img = *img_ptr;

    //to grayscale
    Mat out = img.clone();
    cvtColor(img, img, COLOR_BGR2GRAY);
    medianBlur(img, img, 5);

    vector<Vec3f> circles;
    HoughCircles(img, circles, HOUGH_GRADIENT, 1, dist, p1, p2, minradius, maxradius);
    for(int i = 0; i < circles.size(); i++){
        Vec3i c = circles[i];
        Point center = Point(c[0], c[1]);
        circle(out, center, 1, Scalar(0,100,100), 1, LINE_AA);
        int radius = c[2];
        circle(out, center, radius, Scalar(255,0,255), 1, LINE_AA);
    }

    imshow("detected circles", out);
}

int main(int argc, char** argv){

    Mat img = imread("street_scene.png", IMREAD_COLOR);
    if (img.empty()) {
        std::cout << "ERROR: invalid filepath. Place 'street_scene.png' in the same folder." << std::endl;
        return -1;
    }

    // default found optimal values for detection
    dist = 61; minradius = 5; maxradius=18, p1 = 200; p2=21;
    namedWindow("detected circles");
    createTrackbar("distance", "detected circles", &dist, 100, onTrackbar, static_cast<void*>(&img));
    setTrackbarMin("distance", "detected circles", 1);
    createTrackbar("min  radius", "detected circles", &minradius, 100, onTrackbar, static_cast<void*>(&img));
    setTrackbarMin("min  radius", "detected circles", 1);
    createTrackbar("max radius", "detected circles", &maxradius, 300, onTrackbar, static_cast<void*>(&img));
    createTrackbar("param 1", "detected circles", &p1, 200, onTrackbar, static_cast<void*>(&img));
    setTrackbarMin("param 1", "detected circles", 1);
    createTrackbar("param 2", "detected circles", &p2, 200, onTrackbar, static_cast<void*>(&img));
    setTrackbarMin("param 2", "detected circles", 1);

    waitKey(0);
    destroyAllWindows();

    return 0;
}