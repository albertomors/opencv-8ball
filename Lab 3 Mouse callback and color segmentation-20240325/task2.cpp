/*
Task 2
Expand Task 1 adding a callback function reacting to the mouse click.
A callback is a function whose execution is triggered by an event, in our case, a mouse click.
This is managed by the GUI–in OpenCV you can use the function setMouseCallback()1
that sets a callback on a specific mouse event. Your callback shall print the BGR color triplet
of the pixel where the click occurred.
*/

#include <opencv2/highgui.hpp>
#include <iostream>

void onMouse(int event, int x, int y, int flags, void* userdata){
    //cast void* to cv::Mat* to get &img
    cv::Mat* img_ptr = static_cast<cv::Mat*>(userdata);
    cv::Mat img = *img_ptr; //get img by *(&img)

    if(event == cv::EVENT_LBUTTONDOWN){
        std::cout << img.at<cv::Vec3b>(y,x) << std::endl;
        //-> to access members of a pointer to an object, . to access members of an actual object
    }
}

int main(int argc, char** argv){

    //load image
    cv::Mat img = cv::imread("robocup.jpg", cv::IMREAD_ANYCOLOR);
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
