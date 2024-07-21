/*
    AUTHOR: 
    DATE: 2024-07-21
    FILE: main.cpp
    DESCRIPTION: Main entry point for the application that handles video processing. It initializes the video handler and processes the video based on command line arguments.

    FUNCTIONS:
    - int main(int argc, char** argv): Main function that processes command line arguments, initializes the video handler, and processes the video.

    PARAMETERS:
    - argc: The number of command line arguments.
    - argv: An array of command line arguments.

    EXAMPLES:
    - Input: Command line arguments specifying the folder name and a flag to view or not view the algorithm's mid-steps.
    - Output: Processes the video and provides feedback on the completion status.

    USAGE:
    - Example: ./main game1_clip1 y
      This command runs the program on the folder "game1_clip1" with the flag to view the algorithm's mid-steps.

    NOTES:
    - The program requires at least two command line arguments: the folder name and a flag to indicate whether to view the mid-steps of the algorithm.
    - The program uses the videoHandler class to process the video and handles errors appropriately.
*/

#include "videoHandler.h"

cv::Mat plot_bb(const cv::Mat& src, const cv::Mat& bb);

int main(int argc, char** argv) {

    if (argc<3) {
        std::cerr << "Error: Missing cmd line arguments! Pass folder_name followed by FLAG(y/n) to view or not algorithm's mid-steps" << std::endl;
        std::cout << "Example> ./main game1_clip1 y" << std::endl;
        return -1;
    }
    
    std::string folder_name = argv[1];
    bool MIDSTEP_flag = (std::tolower(argv[2][0]) == 'y');

    videoHandler handler = videoHandler(folder_name);
    handler.process_video(MIDSTEP_flag);

    if (handler.errors == false){
        std::cout << "Terminated without errors." << std::endl;
        return 0;
    }

    std::cerr << "Process terminated due errors." << std::endl;
    return -1;
}