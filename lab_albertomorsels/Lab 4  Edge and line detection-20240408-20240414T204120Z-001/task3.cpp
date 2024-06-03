
/*
Task 3
Now detect white markings using the Hough transform. Check online sources and apply it
using the cv::HoughLines() function. Suggestion: consider the two strongest lines detected,
and select their orientation. Color in red the area between the lines - example below.
*/

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int a1,a2,t;
int htresh;

void onTrackbar(int pos, void* userdata){

    cv::Mat* img_ptr = static_cast<cv::Mat*>(userdata);
    cv::Mat img = *img_ptr;

    //to grayscale
    Mat out = img.clone();
    cvtColor(img, img, COLOR_BGR2GRAY);

    //canny
    cv::Mat edges;
    blur(img, img, cv::Size(3,3));
    Canny(img, edges, 50, htresh);

    //hough-transform
    vector<Vec2f> lines; //store (rho, theta) of detected lines
    HoughLines(edges, lines, 1, CV_PI/180, htresh);

    //sort by length (rho) -- criteria of sorting (first component == rho)
    sort(lines.begin(), lines.end(), [](const Vec2f& l1, const Vec2f& l2) { return l1[0] > l2[0]; });

    //filter by direction
    Vec2f line1, line2;
    cv::Point pts[3];
    for(int i = 0; i < lines.size(); i++){
        float rho = lines[i][0], theta = lines[i][1];

        //filter by 1st angle
        if( (abs(theta*180/CV_PI-a1)<t)){
            Point pt1, pt2;
            double a = cos(theta), b = sin(theta);
            double x0 = a*rho, y0 = b*rho;
            pt1.x = cvRound(x0 + 1000*(-b));
            pt1.y = cvRound(y0 + 1000*(a));
            pt2.x = cvRound(x0 - 1000*(-b));
            pt2.y = cvRound(y0 - 1000*(a));
            line(out, pt1, pt2, Scalar(0,0,255), 1, LINE_AA);
            line1 = lines[i];
            pts[0] = pt1;
        }

        //filter by 2nd angle
        if( (abs(theta*180/CV_PI-a2)<t)){
            Point pt1, pt2;
            double a = cos(theta), b = sin(theta);
            double x0 = a*rho, y0 = b*rho;
            pt1.x = cvRound(x0 + 1000*(-b));
            pt1.y = cvRound(y0 + 1000*(a));
            pt2.x = cvRound(x0 - 1000*(-b));
            pt2.y = cvRound(y0 - 1000*(a));
            line(out, pt1, pt2, Scalar(0,0,255), 1, LINE_AA);
            line2 = lines[i];
            pts[1] = pt2;
        }
    }

    if(line1 != cv::Vec2f(0.0f, 0.0f) && line2 != cv::Vec2f(0.0f, 0.0f)){
        float rho1 = line1[0], theta1 = line1[1];
        float rho2 = line2[0], theta2 = line2[1];

        float a1 = cos(theta1), b1 = sin(theta1);
        float a2 = cos(theta2), b2 = sin(theta2);

        float determinant = a1 * b2 - a2 * b1;
        float x = (b2 * rho1 - b1 * rho2) / determinant;
        float y = (a1 * rho2 - a2 * rho1) / determinant;
        pts[2].x = cvRound(x);
        pts[2].y = cvRound(y);

        cv::fillConvexPoly(out, pts, 3, cv::Scalar(0, 0, 255));
    }
    
    imshow("houghlines", out);
}

int main(int argc, char** argv){

    Mat img = imread("street_scene.png", IMREAD_COLOR);
    if (img.empty()) {
        std::cout << "ERROR: invalid filepath. Place 'street_scene.png' in the same folder." << std::endl;
        return -1;
    }

    //41 128 6 150 default found optimal values for detection
    a1 = 41; a2 = 128; t = 6; htresh = 150;
    cv::namedWindow("houghlines");
    cv::createTrackbar("alpha 1 [deg]", "houghlines", &a1, 360, onTrackbar, static_cast<void*>(&img));
    cv::createTrackbar("alpha 2 [deg]", "houghlines", &a2, 360, onTrackbar, static_cast<void*>(&img));
    cv::createTrackbar("tolerance [deg]", "houghlines", &t, 20, onTrackbar, static_cast<void*>(&img));
    cv::createTrackbar("hough tresh", "houghlines", &htresh, 255, onTrackbar, static_cast<void*>(&img));

    cv::waitKey(0);
    cv::destroyAllWindows();

    return 0;
}