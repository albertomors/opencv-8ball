/*
AUTHOR: Morselli Alberto 
FILE: videoHandler.cpp

-given the folder path extract the frames and pass them to frameHandler
-show frame by frame results if flag is true, otherwise just first and final
-receive frames from frameHandler to build the final video
-call metrics at the end
*/

#include "videoHandler.h"
#include "frameHandler.h"
#include "metrics.h"

videoHandler::videoHandler(const std::string& folder_name) {
    this->errors = false;
    this->folder_name = folder_name;
    this->load_files();
}

void videoHandler::load_files() {
    std::string mask_path = "../res/Dataset/" + folder_name + "/masks/";
    this->ffirst_mask = cv::imread(mask_path + "frame_first.png", cv::IMREAD_GRAYSCALE);
    this->flast_mask = cv::imread(mask_path + "frame_last.png", cv::IMREAD_GRAYSCALE);
    if (ffirst_mask.empty() || flast_mask.empty()){
        std::cerr << "Error: Could not load the seg-mask groundtruth files!" << std::endl;
        std::cout << "Ensure to be inside the build folder with the shell and pass a valid name for the folder." << std::endl;
        this->errors = true;
        return;
    }

    std::string bb_path = "../res/Dataset/" + folder_name + "/bounding_boxes/";
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
    std::vector<int> data;
    int value;
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
    std::string folder_path = "../res/Dataset/" + folder_name;
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

    //create output folder inside build inside the root project path
    std::string build_folder = "../build";
    if (!cv::utils::fs::exists(build_folder)) {
        cv::utils::fs::createDirectories(build_folder);
        std::cout << "Succesfully created folder: " << build_folder << std::endl;
    }
    std::string out_folder = "../build/output";
    if (!cv::utils::fs::exists(out_folder)) {
        cv::utils::fs::createDirectories(out_folder);
        std::cout << "Succesfully created folder: " << out_folder << std::endl;
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
    cv::Mat frame_i, ret_frame;
    frameHandler frame_handler = frameHandler();

    while (i <= tot_frames) {
        capture >> frame_i;
        std::cout << "frame " << i << "/" << tot_frames << std::endl;

        //elaborate video - call frameHandler --------------------

        //runs only for first frame or every if MIDSTEP_flag==true
        if (i==1 || MIDSTEP_flag){
            frame_handler.detect_table(frame_i);
            frame_handler.detect_balls(frame_i);

            //get_seg_masks and bb TODO
            cv::Mat frame_i_ret_bb = this->ffirst_bb.clone(); //fantoccio per returned
            cv::Mat frame_i_ret_mask = this->ffirst_mask.clone();

            //instead this runs at every frame if MIDSTEP_flag==true
            cv::namedWindow("bb"); cv::imshow("bb", this->plot_bb(frame_i, frame_i_ret_bb));
            cv::namedWindow("mask"); cv::imshow("mask", this->displayMask(frame_i_ret_mask));
            ret_frame = frame_handler.draw_frame(frame_i);
            cv::namedWindow("frame_i"); cv::imshow("frame_i", ret_frame);   

            //in case you want to visualize all steps this saves the masks ONLY for first frame
            if (i==1){
                frame_handler.initializeTrackers(frame_i);

                this->ffirst_ret_bb = this->ffirst_bb.clone();
                this->ffirst_ret_mask = this->ffirst_mask.clone();
                cv::waitKey(0);
            }
        }

        //runs for every frame
        frame_handler.updateTrackers(frame_i);
        frame_handler.project(frame_i);

        ret_frame = frame_handler.draw_frame(frame_i);
        cv::namedWindow("frame_i"); cv::imshow("frame_i", ret_frame);   
        cv::waitKey(1);

        //runs only for last frame
        if(i==tot_frames-1){
            //get_seg_masks and bb
            this->flast_ret_bb = this->flast_bb.clone();
            this->flast_ret_mask = this->flast_mask.clone();

            cv::namedWindow("mask"); cv::imshow("mask", this->displayMask(this->flast_ret_mask));
            cv::namedWindow("bb"); cv::imshow("bb", this->plot_bb(frame_i, this->flast_ret_bb));
            cv::waitKey(0);
        }

        //-------------------------------------------------------
        
        writer.write(ret_frame);
        i++;
    }

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

    std::cout << "---METRICS-------------" << std::endl;
    double mAP = compute_mAP(this->ffirst_bb,this->ffirst_ret_bb) + compute_mAP(this->flast_bb,this->flast_ret_bb);
    std::cout << "mAP = " << mAP/2.0 << std::endl;
    
    //qua provo solo a sporcare un po' la maschera di seg con due rettangoli
    std::vector<std::pair<cv::Mat, cv::Mat>> segmasks;
    cv::rectangle(this->ffirst_ret_mask,cv::Rect(300,300,50,50),cv::Scalar(3),cv::FILLED);
    cv::rectangle(this->ffirst_ret_mask,cv::Rect(657,342,23,102),cv::Scalar(1),cv::FILLED);

    std::pair<cv::Mat,cv::Mat> pairfirst = std::make_pair(this->ffirst_mask,this->ffirst_ret_mask);
    std::pair<cv::Mat,cv::Mat> pairlast = std::make_pair(this->flast_mask,this->flast_ret_mask);
    segmasks.push_back(pairfirst);
    segmasks.push_back(pairlast);
    
    double mIoU = compute_mIoU(segmasks,6);
    std::cout << "mIoU = " << mIoU << std::endl;
}

//-----------------------------------------------------------

cv::Mat videoHandler::displayMask(const cv::Mat& mask){
    cv::Mat bgr;
    cv::cvtColor(mask, bgr, cv::COLOR_GRAY2BGR);

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
    cv::LUT(bgr, lookUpTable, colored);
    
    return colored;
}

cv::Mat videoHandler::plot_bb(const cv::Mat& src, const cv::Mat& bb){
    cv::Mat edit = src.clone();

    //build LUT to convert class values [0..5] to BGR colors
    cv::Mat lookUpTable = cv::Mat::zeros(1, 256, CV_8UC3);
    //lookUpTable.at<cv::Vec3b>(0) = cv::Vec3b(128,128,128);
    lookUpTable.at<cv::Vec3b>(1) = cv::Vec3b(255,255,255);
    lookUpTable.at<cv::Vec3b>(2) = cv::Vec3b(0,0,0);
    lookUpTable.at<cv::Vec3b>(3) = cv::Vec3b(0,0,255);
    lookUpTable.at<cv::Vec3b>(4) = cv::Vec3b(255,0,0);
    //lookUpTable.at<cv::Vec3b>(5) = cv::Vec3b(0,255,0);

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
    uint16_t x,y,w,h,c;
    for(int i=0; i<bb.rows; ++i){
        x = bb.at<uint16_t>(i,0);
        y = bb.at<uint16_t>(i,1);
        w = bb.at<uint16_t>(i,2);
        h = bb.at<uint16_t>(i,3);
        c = bb.at<uint16_t>(i,4);
        cv::Rect rect(x,y,w,h);

        cv::rectangle(edit, rect, lookUpTable.at<cv::Vec3b>(c), 1);
    }
    
    return edit;
}