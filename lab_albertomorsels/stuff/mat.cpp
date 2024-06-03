//https://docs.opencv.org/4.x/d6/d6d/tutorial_mat_the_basic_image_container.html

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, char** argv){

    /*
    Mat img(400, 400, CV_8UC1);
    cout << img.depth() << " " << img.channels() << endl;
    cout << img.cols << " " << img.rows << endl;

    for (int i = 0; i < img.cols; ++i)
        for (int j = 0; j < img.rows; ++j)
            img.at<uchar> (i, j) = i+j; //when exceed 255 periodically restarst from 0 [circle]

    namedWindow("img"); //shows that took effect also on img
    imshow ("img", img);
    waitKey(0);
    destroyWindow("img");

    //Mat img2(img); //copy constructor - points to the same
    Mat img2 = img;
    //alter img2
    for (int i = 0; i < img.cols; ++i){
        for (int j = img.rows*2/3; j < img.rows; ++j)
            img.at<uchar> (i, j) = 0;
    }

    Mat D (img, Rect(0, 0, 100, 100) );   // using a rectangle
    Mat E = img(Range::all(), Range(100,300));  // using row and column boundaries
    cout << E.rows << E.cols << endl;

    for (int i = 0; i < E.cols; ++i){
        for (int j = 0; j < E.rows; ++j)
            img.at<uchar> (i, j) = 255;
    }
    
    namedWindow("D");
    imshow ("D", D);
    namedWindow("E");
    imshow ("E", E);

    namedWindow("img"); //shows that took effect also on img
    imshow ("img", img);
    waitKey(0);
    destroyAllWindows();

    //to copy
    Mat F = img.clone();
    Mat G;
    img.copyTo(G);
    */

    Mat M(2,2, CV_8UC3, Scalar(0,0,255));
    M.at<uchar>(0,0) = 128; //if you index it with the wrong type you de-compose the triplets and access to the first BGR (=B)
    M.at<Vec3b>(1,1) = Vec3b(128,128,128);
    cout << "M = " << endl << " " << M << endl << endl;

    Mat L(3,3, CV_8UC3, Scalar::all(128)); //CV_8UC(1) is equivalent. Scalar::all(n) initialize all pixel triplet to 0
    cout << "L = " << endl << " " << L << endl << endl;

    Mat E = Mat::eye(4, 4, CV_64F);
    cout << "E = " << endl << " " << E << endl << endl;
    Mat O = Mat::ones(2, 2, CV_32F);
    cout << "O = " << endl << " " << O << endl << endl;
    Mat Z = Mat::zeros(3,3, CV_8UC1);
    cout << "Z = " << endl << " " << Z << endl << endl;

    //random
    Mat R = Mat(400, 400, CV_8UC1);
    randu(R, Scalar::all(0), Scalar::all(255));
    cvtColor(R,R,COLOR_GRAY2BGR);
    cout << R.channels() << endl;
    namedWindow("img"); //shows that took effect also on img
    imshow ("img", R);
    waitKey(0);
    destroyAllWindows();

    return 0;
}