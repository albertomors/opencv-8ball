//https://docs.opencv.org/4.x/db/da5/tutorial_how_to_scan_images.html

#include <opencv2/highgui.hpp>
#include <iostream>

using namespace std;
using namespace cv;

int main( int argc, char** argv){
    
    Mat I = imread(argv[1], IMREAD_COLOR);
    if(I.empty()){
        cout << "Could not open or find the image!\n" << endl;
        return -1;
    }
    CV_Assert(I.type() == CV_8UC(3));

    float gamma = (argc > 2) ? stof(argv[2]) : 1.0;
    int beta = (argc > 3) ? stoi(argv[3]) : 0;
    CV_Assert(gamma >= 0 && beta >= -numeric_limits<uchar>::max() and beta <= numeric_limits<uchar>::max());

    /* Mat lookUpTable(1, 256, CV_8UC(3));
    for (int i=0; i<256; ++i)
        for (int c=0; c<I.channels(); ++c)
            lookUpTable.at<cv::Vec3b>(i)[c] = saturate_cast<uchar>(pow(i / 255.0, gamma) * 255.0);
    cout << lookUpTable << endl; */

    //affine contrast stretching
    Mat lookUpTable(1, 256, CV_8UC(3));
    for (int i=0; i<256; ++i)
        for (int c=0; c<I.channels(); ++c)
            lookUpTable.at<cv::Vec3b>(i)[c] = saturate_cast<uchar>(gamma*i + beta);
    cout << lookUpTable << endl;
        
    Mat J = I.clone();
    LUT(I, lookUpTable, J);

    namedWindow("I"); imshow ("I", I);
    namedWindow("J"); imshow ("J", J);
    waitKey(0); destroyAllWindows();
    
    return 0;
}