/*
AUTHOR: Morselli Alberto
FILE: main.cpp

call videoHandler
*/

#include "videoHandler.h"

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