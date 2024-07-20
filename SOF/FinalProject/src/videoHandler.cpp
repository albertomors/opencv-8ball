/*
AUTHOR: Morselli Alberto 
FILE: videoHandler.cpp

-given the folder path extract the frames and pass them to frameHandler
-show frame by frame results if flag is true, otherwise just first and final
-receive frames from frameHandler to build the final video
-call metrics at the end
*/

#include "videoHandler.h"
#include "metrics.h"
#include "table.h"
#include "ballDetection.h"
#include "trajectoryTracking.h"
#include "trajectoryProjection.h"

videoHandler::videoHandler(const std::string& folder_name) {
    this->errors = false;
    this->folder_name = folder_name;
    this->load_files();
}

void videoHandler::load_files() {
    std::string mask_path = "Dataset/" + folder_name + "/masks/";
    this->ffirst_mask = cv::imread(mask_path + "frame_first.png");
    this->flast_mask = cv::imread(mask_path + "frame_last.png");
    if (ffirst_mask.empty() || flast_mask.empty()){
        std::cerr << "Error: Could not load the seg-mask groundtruth files!" << std::endl;
        std::cout << "Ensure to be inside the build folder with the shell and pass a valid name for the folder." << std::endl;
        this->errors = true;
        return;
    }

    std::string bb_path = "Dataset/" + folder_name + "/bounding_boxes/";
    this->ffirst_bb = load_txt_data(bb_path + "frame_first_bbox.txt");
    this->flast_bb = load_txt_data(bb_path + "frame_last_bbox.txt");
}

cv::Mat videoHandler::load_txt_data(const std::string& path){
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << path << "." << std::endl;
        this->errors = true;
        return cv::Mat{};
    }

    //process file and store data in a vector
    std::vector<uint16_t> data;
    uint16_t value;
    while (file >> value)
        data.push_back(value);
    file.close();

    //create data matrix
    int rows = data.size() / 5;
    int cols = 5;
    cv::Mat data_matrix(rows, cols, CV_16U);

    //fill-in matrix
    int idx = 0;
    for (int i=0; i<rows; ++i) {
        for (int j=0; j<cols; ++j) {
            data_matrix.at<uint16_t>(i,j) = data[idx++];
    }}

    return data_matrix;
}

void videoHandler::process_video(int MIDSTEP_flag){
    std::string folder_path = "Dataset/" + folder_name;
    std::string video_path = folder_path + "/" + folder_name + ".mp4";

    cv::VideoCapture capture(video_path);

    if (!capture.isOpened()) {
        std::cerr << "Error: Could not load the seg-mask groundtruth files!" << std::endl;
        std::cout << "Ensure to be inside the build folder with the shell and pass a valid name for the folder." << std::endl;
        this->errors = true;
        return;
    }

    //get video properties
    int codec = capture.get(cv::CAP_PROP_FOURCC);
    double fps = capture.get(cv::CAP_PROP_FPS);
    int tot_frames = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_COUNT));
    int frame_width = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_WIDTH));
    int frame_height = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_HEIGHT));
    cv::Size frame_size(frame_width, frame_height);

    std::cout << "codec: " << codec << std::endl << "fps: " << fps << std::endl << "tot_frames: " << tot_frames << std::endl << "frame_size: " << frame_size << std::endl;

    //create results folder in the root project path
    std::string out_folder = "results";
    if (!cv::utils::fs::exists(out_folder)) {
        if (!cv::utils::fs::createDirectories(out_folder))
            std::cout << "Succesfully created folder: " << out_folder << std::endl;
        else{
            std::cerr << "Error: Impossible to create folder: " << out_folder << std::endl;
            this->errors = true;
            return;
        }
    }

    // Define the output video path
    std::string out_path = out_folder + "/" + folder_name + ".mp4";
    cv::VideoWriter writer(out_path, codec, fps, frame_size, true);

    if (!writer.isOpened()) {
        std::cerr << "Error in creating video writer." << std::endl;
        this->errors = true;
        return;
    }

    int i = 1;
    cv::Mat frame_i;
    tableDetector table = tableDetector();
    ballDetector balls_detector = ballDetector();
    trajectoryTracker tracker = trajectoryTracker();
    trajectoryProjecter projecter = trajectoryProjecter();

    while (i <= tot_frames) {
        capture >> frame_i;
        
        //elaborate video - call frameHandler --------------------
        
        table.find_table(frame_i);
        cv::Mat drawed = table.draw_borders(frame_i);
        if (MIDSTEP_flag){
            std::cout << "frame " << i << "/" << tot_frames << std::endl;
            cv::namedWindow("frame_i"); cv::imshow("frame_i", drawed);
            cv::namedWindow("seg_mask"); cv::imshow("seg_mask", table.seg_mask);
            cv::namedWindow("ROI"); cv::imshow("ROI", table.seg_mask);
            cv::waitKey(1);
        }

        bool RET_flag = (i == 1 || i == tot_frames) ? true : false;
        cv::Mat mask;
        if(RET_flag)
            mask = (i == 1) ? ffirst_mask : flast_mask;

        // -------------------------------------------------------

        if(RET_flag){ //returned masks and midsteps
            cv::Mat ret_mask = mask;
            cv::namedWindow("mask"); cv::imshow("mask", displayMask(ret_mask));
            cv::waitKey();
            cv::destroyWindow("mask");
        }

        balls_detector.detectBalls(frame_i, table.seg_mask, table.color);

        if(i==1){
            projecter.findLines(frame_i, table.hull);
            projecter.projectBalls(frame_i, tracker.centers, tracker.trajectories, balls_detector.id_balls);
            tracker.initializeTrackers(frame_i, balls_detector.balls);
        }
        else{
            tracker.updateTrackers(frame_i);
            projecter.projectBalls(frame_i, tracker.centers, tracker.trajectories, balls_detector.id_balls);
        }

        writer.write(drawed);
        i++;
    }

    cv::imshow("before", frame_i);
    cv::waitKey(1);

    //tracker.drawTrajectories(frame_i);
    cv::waitKey(1);

    std::cout << "Process exited from loop after " << i-1 << " frames elaborated." << std::endl;
    capture >> frame_i;
    CV_Assert(frame_i.empty()); //check that video was actually finished

    capture.release();
    writer.release();
    std::cout << "Video saved at " << out_path << "." << std::endl;

    /*
    call to
    compute metrics
    ...
    */
}






//-----------------------------------------------------------

cv::Mat displayMask(const cv::Mat& mask){

    //build LUT to convert mask values [0..5] to BGR colors
    cv::Mat lookUpTable = cv::Mat::zeros(1, 256, CV_8UC3);
    lookUpTable.at<cv::Vec3b>(0) = cv::Vec3b(128,128,128);
    lookUpTable.at<cv::Vec3b>(1) = cv::Vec3b(255,255,255);
    lookUpTable.at<cv::Vec3b>(2) = cv::Vec3b(0,0,0);
    lookUpTable.at<cv::Vec3b>(3) = cv::Vec3b(0,0,255);
    lookUpTable.at<cv::Vec3b>(4) = cv::Vec3b(255,0,0);
    lookUpTable.at<cv::Vec3b>(5) = cv::Vec3b(0,255,0);

    /* 
    0. Background
    1. white "cue ball"
    2. black "8-ball"
    3. ball with solid color
    4. ball with stripes
    5. playing field (table)

    0: (128,128,128)
    1: (255, 255, 255)
    2: (0,0,0)
    3: (0,0,255)
    4: (255,0,0,)
    5: (0,255,0)
    */

    //actually converts and display it
    cv::Mat colored;
    cv::LUT(mask, lookUpTable, colored);
    
    return colored;
}