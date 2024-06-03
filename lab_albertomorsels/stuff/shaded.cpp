#include <opencv2/highgui.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, char** argv){

    Mat img(400, 400, CV_8UC1);
    cout << img.depth() << " " << img.channels() << endl;
    cout << img.cols << " " << img.rows << endl;

    for (int i = 0; i < img.cols; ++i)
        for (int j = 0; j < img.rows; ++j)
            img.at<uchar> (i, j) = i+j; //when exceed 255 periodically restarst from 0 [circle]

    namedWindow("shaded");
    imshow ("shaded", img);
    waitKey(0);

    return 0;
}