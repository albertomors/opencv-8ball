/*
Task 6
Expand Task 4 by creating a new image whose pixels are:
● equal to the input image if the corresponding pixel in the mask is black;
● equal to a given color otherwise.
Choose the triplet (92, 37, 201) as the new color. This image is generated from scratch at
every new click on the original image.
Are you able to change the color of the T-shirts?
Are you able to change the color of the ball?
*/

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include <iostream>

cv::Vec3b get_avg_color(const cv::Mat& img, int x,int y){
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

    return avg/(9*9);
}

bool elemDistTresh(const cv::Vec3b& v1, const cv::Vec3b& v2, const cv::Vec3b& treshold){
    for (int i=0; i<3; ++i){
        if(std::abs(v1[i] - v2[i]) > treshold[i])
            return false;
    }
    return true; //iff pass all tests
}

cv::Mat create_mask(const cv::Mat& src, cv::Vec3b avg, int T){
    cv::Mat mask(src.size(), src.type());
    
    for (int y=0; y<mask.rows; ++y){
        for (int x=0; x<mask.cols; ++x){
            mask.at<cv::Vec3b>(y,x) = elemDistTresh(src.at<cv::Vec3b>(y,x), avg, cv::Vec3b(T,T,T)) ? cv::Vec3b(92,37,201) : src.at<cv::Vec3b>(y,x);
            //write pixel value = (if elementwise abs diff < T) then white : otherwise black
    }}
    return mask;
}

void onMouse(int event, int x, int y, int flags, void* userdata){
    std::pair<cv::Mat, int>* params_ptr = static_cast<std::pair<cv::Mat, int>*>(userdata);  //cast void* to std::pair*
    cv::Mat img = params_ptr->first;  //un-pack
    int T =       params_ptr->second;
    //use -> to access members of a pointer to an object

    if(event == cv::EVENT_LBUTTONDOWN){
        cv::Vec3b avg = get_avg_color(img, x,y);
        cv::Mat mask = create_mask(img, avg, T);
        cv::namedWindow("mask");
        cv::imshow("mask", mask);
    }
}

int main(int argc, char** argv){

    cv::Mat img = cv::imread("robocup.jpg", cv::IMREAD_COLOR);
    if (img.empty()) {
        std::cout << "ERROR: invalid filepath. Place 'robocup.jpg' in the same folder." << std::endl;
        return -1; }
    if (argc < 2){
        std::cout << "ERROR: missing treshold T. Pass it as argv[1] between 0 and 255." << std::endl;
        return -1; }
    
    int T = std::stoi(argv[1]); //string-to-int
    if (T<0 || T>255){
        std::cout << "ERROR: invalid treshold T. Has to be a value between 0 and 255." << std::endl;
        return -1; }

    cv::namedWindow("robocup.jpg");
    cv::imshow("robocup.jpg", img);

    std::pair<cv::Mat, int> params(img, T);
    cv::setMouseCallback("robocup.jpg", onMouse, static_cast<void*>(&params));
    cv::waitKey(0);
    cv::destroyAllWindows();

    return 0;
}
