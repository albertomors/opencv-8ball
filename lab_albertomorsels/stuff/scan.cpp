//https://docs.opencv.org/4.x/db/da5/tutorial_how_to_scan_images.html
#include <opencv2/highgui.hpp>

using namespace cv;

int main( int argc, char* argv[])
{
    Mat I, J;
    I = imread(argv[1], IMREAD_COLOR);
    CV_Assert(I.type() == CV_8UC3);

    cv::Mat table(1, 256, CV_8UC(3));
    for (int i=0; i<256; i++)
        for (int c=0; c<I.channels(); ++c)
            table.at<cv::Vec3b>(i)[c] = 255 - i;

    LUT(I, table, J);

    namedWindow("I");
    imshow ("I", I);
    namedWindow("J");
    imshow ("J", J);
    waitKey(0);
    destroyAllWindows();
    
    return 0;
}