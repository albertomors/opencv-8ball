/*
Task 5
Expand Task 1 plotting the histogram of Garden_grayscale using 256 bins and range [0,
255]. Try also different number of bins. You are free to evaluate the histogram using your
code or the functions provided by OpenCV.
*/

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

cv::Mat calc_hist_img(const cv::Mat& src, int bins){
    //compute histogram
    cv::Mat hist;               //matrix where rows represent level [0-255] with 32F values for bins level
    float range[] = {0, 256};   //upper is exclusive
    const float* hist_range = {range};
    bool uniform = true, accumulate = false; //bins have the same size, reset at every call

    cv::calcHist(&src, 1, 0, cv::Mat(), hist, 1, &bins, &hist_range, uniform, accumulate);
    cv::normalize(hist, hist, 0, 255, cv::NORM_MINMAX);
    hist.convertTo(hist, CV_8UC1); //adapt <float 32F> values to <uchar>
    
    //build the image
    int hist_h = 256;
    int bins_w = std::max(1,hist_h/bins); //rounded to int
    int hist_w = bins_w*bins; //to assure integer values

    cv::Mat hist_img = cv::Mat(hist_h, hist_w, hist.type(), cv::Scalar(255)); //white background
    for(int i=0; i<hist.rows; ++i)
        for(int v=0; v<=int(hist.at<uchar>(i)); ++v)
            for(int b=0; b<bins_w; ++b)
                hist_img.at<uchar>(255-v,i*bins_w+b) = 0; //paint black;
    
    return hist_img;
}

int main(int argc, char** argv){
    if (argc<2 || std::stoi(argv[1]) < 1 || std::stoi(argv[1]) > 255){
        std::cout << "ERROR: pass the bins values (1--255) as argv[1] on the cmd-line. " << std::endl;
        return -1;
    }
    int bins = std::stoi(argv[1]);

    //load image
    cv::Mat img = cv::imread("Images/Garden.jpg", cv::IMREAD_COLOR);
    if (img.empty()) {
        std::cout << "Image not found at specified path. Please provide a valid one recompiling task1.cpp \
        or place Garden.jpg in the correct position. [../Images/] respect to this file." << std::endl;
        return -1;
    }
    assert(img.type() == CV_8UC3);

    //converts and shows converted
    cv::Mat out;
    cv::cvtColor(img, out, cv::COLOR_BGR2GRAY);
    assert(out.type() == CV_8UC1);
    cv::namedWindow("grayscale"); cv::imshow("grayscale", out);

    cv::Mat hist_img = calc_hist_img(out,bins);
    cv::namedWindow("histogram"); cv::imshow("histogram", hist_img);
    std::cout << "Press any key to continue..." << std::endl;
    cv::waitKey(0);

    cv::destroyAllWindows();
    cv::imwrite("Results/histogram"+std::to_string(bins)+".png",hist_img);
    std::cout << "Results saved at 'Results/'." << std::endl;

    return 0;
}