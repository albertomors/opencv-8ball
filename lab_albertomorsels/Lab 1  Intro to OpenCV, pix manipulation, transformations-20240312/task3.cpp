/*
Task 3
In a new file, edit the software in Task 2 by adding a function that checks if the number of
channels of the input image is 3. If so, it sets to 0 the first channel and visualizes the image.
Which color is missing? Try other versions that set to 0 the second or the third channel.
Which color is missing? What is the color coding used by OpenCV? Try with all the images
provided.
*/

#include <opencv2/highgui.hpp>
#include <iostream>

int main(int argc, char** argv)
{
    if(argc<2){
        std::cout << "please provide a filepath through cmd-line" << std::endl;
        return -1;
    }
    cv::Mat img = cv::imread(argv[1], cv::IMREAD_ANYCOLOR);

    if(img.empty()){ //img.empty() <=> img.data == NULL
        std::cout << "please provide a VALID filepath" << std::endl;
        return -1;
    }

    int ch = img.channels();
    std::cout << "channels: " << ch << std::endl;

    int c = (argc > 2) ? std::stoi(argv[2]) : 0;
    if(ch == 3){
        for(int y=0; y<img.rows; ++y){
            for(int x=0; x<img.cols; ++x){
                img.at<cv::Vec3b>(y,x)[c] = 0;
        }}

        cv::namedWindow(argv[1]); cv::imshow(argv[1], img);
        char key = cv::waitKey(0);
        std::cout << "key pressed: " << key << std::endl;
    }

    return 0;
}