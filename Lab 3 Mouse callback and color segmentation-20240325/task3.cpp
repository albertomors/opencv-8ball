/*
Task 3
Modify the callback in Task 2: the new version calculates the mean of the B, G and R values
(separately) in a 9x9 neighborhood around the clicked pixel and prints such values.
*/

#include <opencv2/highgui.hpp>
#include <iostream>

void onMouse(int event, int x, int y, int flags, void* userdata){
    //cast void* to cv::Mat* to get &img
    cv::Mat* img_ptr = static_cast<cv::Mat*>(userdata);
    cv::Mat img = *img_ptr; //get img by *(&img)

    if(event == cv::EVENT_LBUTTONDOWN){

        //compute borders for a 9x9 patch
        int y1 = std::max(0,          y-4);
        int y2 = std::min(img.rows-1, y+4);
        int x1 = std::max(0,          x-4);
        int x2 = std::min(img.cols-1, x+4);

        cv::Vec3i avg(0,0,0); //to avoid top limit of 255
        //PLEASE NOTE: redefine x,y variables
        for (y=y1; y<=y2; ++y){
            for (x=x1; x<=x2; ++x){
                avg += img.at<cv::Vec3b>(y,x);
        }}
        std::cout << avg/(9*9) << std::endl;
    }
}

int main(int argc, char** argv){

    //load image
    cv::Mat img = cv::imread("robocup.jpg", cv::IMREAD_COLOR);
    if (img.empty()) {
        std::cout << "ERROR: invalid filepath. Place 'robocup.jpg' in the same folder." << std::endl;
        return -1;
    }

    cv::namedWindow("robocup.jpg");
    cv::imshow("robocup.jpg", img);

    cv::setMouseCallback("robocup.jpg", onMouse, static_cast<void*>(&img)); //cast &(address) of the object img to a void*
    cv::waitKey(0);
    cv::destroyAllWindows();

    return 0;
}
