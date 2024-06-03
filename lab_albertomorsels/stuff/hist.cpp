//https://docs.opencv.org/4.x/db/da5/tutorial_how_to_scan_images.html

#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
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
    Mat J;

    vector<Mat> BGR;
    split(I, BGR);
    for(int i=0; i<3; i++){
        equalizeHist(BGR[i],BGR[i]);
    }
    merge(BGR,J);

    imshow("I",I); imshow("J",J);
    waitKey(0); destroyAllWindows();
    
    return 0;
}