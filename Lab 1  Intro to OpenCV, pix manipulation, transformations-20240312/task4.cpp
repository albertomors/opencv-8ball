/*
Task 4
Edit the software in Task 2 by adding a function that checks if the number of channels of the
input image is 3. If so, create three images with the same size of the input image, one
channel, containing the values found in the first, second and third channel of the original
image. Visualize such images. Try with all the images provided.
*/

#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>
#include <iostream>

int main(int argc, char** argv)
{   
    //load, check and show
    if(argc<2){
        std::cout << "please provide a filepath through cmd-line" << std::endl;
        return -1; }
    cv::Mat img = cv::imread(argv[1], cv::IMREAD_ANYCOLOR);
    if(img.empty()){
        std::cout << "please provide a VALID filepath" << std::endl;
        return -1; }
    cv::namedWindow(argv[1]); cv::imshow(argv[1], img);

    int ch = img.channels();
    std::cout << "channels: " << ch << std::endl;

    if(ch == 3){
        std::vector<cv::Mat> channels;
        cv::split(img, channels);
        for(int c=0; c<3; ++c){
            cv::imshow(std::to_string(c), channels[c]);
        }

        cv::Mat out;
        cv::merge(channels,out);
        cv::imshow("rec", out);

        char key = cv::waitKey(0);
        std::cout << "key pressed: " << key << std::endl;
    }

    return 0;
}