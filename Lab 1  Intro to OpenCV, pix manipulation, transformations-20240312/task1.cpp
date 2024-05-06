/*
Task 1
Implement the OpenCV Hello world discussed during the lectures, adding:
● a safety check on argc - check that the image is provided as a command-line
argument. If argc < 2, print a message that warns the user that an image filename
shall be provided;
● a safety check on the image returned by cv::imread() - what happens if the filename
is wrong? Check on the cv::imread() documentation and handle such condition
properly.
Use the executable to open the grayscale and color images provided and test the safety
checks above.A
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
    
    cv::Mat img = cv::imread(argv[1]);
    if(img.data == NULL){
        std::cout << "please provide a VALID filepath" << std::endl;
        return -1;
    }

    cv::namedWindow(argv[1]);
    cv::imshow(argv[1], img);
    cv::waitKey(0);
    
    return 0;
}