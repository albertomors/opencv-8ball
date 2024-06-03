/*
Task 2
In a new file, edit the software in Task 1 by adding instructions for printing the number of
channels of the image opened. Also, save the output of cv::waitKey() into a char variable
and print it before exiting. Check the OpenCV documentation (or the slides) if you do not
know how to get the number of channels. Run the executable on all the images provided.
*/

#include <opencv2/highgui.hpp>
#include <iostream>

int main(int argc, char** argv)
{
    //passing comma-line args "main arg1 arg2" st argv[0] = name of the caller (main), argv[1] = arg1, argv[2] = arg2
    //in this way argc = 3, it's always 1+more than the ones we pass
    if(argc<2){
        std::cout << "please provide a filepath through cmd-line" << std::endl;
        return -1;
    }
    
    cv::Mat img = cv::imread(argv[1], cv::IMREAD_UNCHANGED); //default IMREAD_COLOR pass everything to COLOR
    if(img.empty()){ //img.empty() <=> img.data == NULL
        std::cout << "please provide a VALID filepath" << std::endl;
        return -1;
    }
    int ch_num = img.channels();
    std::cout << "channels: " << ch_num << std::endl;

    cv::namedWindow(argv[1]);
    cv::imshow(argv[1], img);
    char key = cv::waitKey(0);
    std::cout << "key pressed: " << key << std::endl;
    
    return 0;
}