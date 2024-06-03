
/*
Task 1
Write a program that segments cracks in the asphalt. You can choose one of the
segmentation techniques discussed in the course, or find a new one available in OpenCV. To
assess the generality of your approach, test your algorithm on the three images provided -
the two segments are: crack vs non-crack. Every pixel of the image shall belong to one of
the two categories.
*/

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace std;
using namespace cv;

// global variables here ---------------------------
int m_id = 3;
int k_id = 100, n_id = 33;
int c = 3, d = 3;
//--------------------------------------------------

void contrastStretching(const Mat& src, Mat& dst, Point p1, Point p2){
    Mat table(1, 256, CV_8UC1);

    float k1 = (p1.x == 0)            ? 0 : (p1.y)/(p1.x);
    float k2 = ((p2.x - p1.x) == 0)   ? 0 : (p2.y - p1.y)/(p2.x - p1.x);
    float k3 = ((255 - p2.x) == 0)    ? 0 : (255.0f - p2.y)/(255.0f - p2.x);
    
    for(int r=0; r<256; ++r){
        if (r <= p1.x)                  {table.at<uchar>(r) = k1 * r;}
        else if (r > p1.x && r <= p2.x) {table.at<uchar>(r) = k2 * (r-p1.x) + p1.y;}
        else                            {table.at<uchar>(r) = k3 * (r-p2.x) + p2.y;}
    }

    LUT(src, table, dst);
}

void sauvolaTresh(const Mat& src, Mat& dst, float k, int n){
    dst = src.clone();

    float R = 128.0f;
    int pad = (n-1)/2;
    Scalar mean, std;

    for(int y=0; y<src.rows; ++y){
        for(int x=0; x<src.cols; ++x){
            Mat rect = src(Rect(Point(max(x-pad,0),max(y-pad,0)), Point(min(x+pad,src.cols), min(y+pad, src.rows))));
            meanStdDev(rect, mean, std);
            float treshold = mean[0] * (1.0f + k * (std[0] / R - 1.0f));
            dst.at<uchar>(y,x) = (float(src.at<uchar>(y,x)) > treshold) ? 0 : 255;
        }
    }
}

bool sortByArea(const pair<int, int>& a, const pair<int, int>& b) {
    return a.second > b.second;
}

void extractLargest(const Mat& src, Mat& dst){
    
    dst = src.clone();
    Mat labels, stats, centroids;
    int numLabels = connectedComponentsWithStats(src, labels, stats, centroids);

    vector<pair<int, int>> components;
    //ignore background [label=0]
    for (int label = 1; label < numLabels; label++) {
        int area = stats.at<int>(label, CC_STAT_AREA);
        components.push_back(make_pair(label, area));
    }
    sort(components.begin(), components.end(), sortByArea);

    dst.setTo(0);
    //first [d] components
    for (int i = 0; i < min(d, static_cast<int>(components.size())); i++) { 
        int label = components[i].first;
        int area = components[i].second;

        dst |= (labels == label);
    }
}

void onTrackbar(int pos, void* userdata){

    Mat* img_ptr = static_cast<Mat*>(userdata);
    Mat img = (*img_ptr).clone();

    int m = (m_id%2==0) ? max(1,m_id-1) : m_id;
    float k = (float(k_id)/100) * (0.5f-0.2f) + 0.2f;
    k = clamp(k, 0.2f, 0.5f);
    int n = (n_id%2==0) ? max(1,n_id-1) : n_id;
    cout << "m:" << m << " k:" << k << " n:" << n << endl;

    medianBlur(img, img, m);
    namedWindow("median blur", WINDOW_NORMAL);
    imshow("median blur", img);

    sauvolaTresh(img, img, k, n);
    namedWindow("local tresholding", WINDOW_NORMAL);
    imshow("local tresholding", img);

    morphologyEx(img, img, MORPH_CLOSE, getStructuringElement(MORPH_ELLIPSE, Size(c,c)));
    namedWindow("closing", WINDOW_NORMAL);
    imshow("closing", img);

    extractLargest(img, img);
    namedWindow("connected domain", WINDOW_NORMAL);
    imshow("connected domain", img);
}

int main(int argc, char** argv){

    if (argc<2) {
        std::cout << "ERROR: missing argument. Pass 1,2 or 3 to select image to process" << std::endl;
        return -1; }
    
    string path = "Asphalt cracks/Asphalt-" + string(argv[1]) + ".png";
    Mat img = imread(path, IMREAD_ANYCOLOR);
    if (img.empty()) {
        std::cout << "ERROR: invalid filepath" << std::endl;
        return -1; }

    namedWindow("original", WINDOW_NORMAL);
    imshow("original", img);

    cvtColor(img, img, COLOR_BGR2GRAY);
    namedWindow("grayscale", WINDOW_NORMAL);
    imshow("grayscale", img);

    equalizeHist(img, img);
    namedWindow("histogram equalization", WINDOW_NORMAL);
    imshow("histogram equalization", img);

    contrastStretching(img, img, Point(0,30), Point(20,230));
    namedWindow("seg contrast stretch", WINDOW_NORMAL);
    imshow("seg contrast stretch", img);
    
    createTrackbar("median", "seg contrast stretch", &m_id, 10, onTrackbar, static_cast<void*>(&img));
    setTrackbarMin("median", "seg contrast stretch", 1);
    createTrackbar("sauvola k", "seg contrast stretch", &k_id, 100, onTrackbar, static_cast<void*>(&img));
    createTrackbar("sauvola n", "seg contrast stretch", &n_id, 100, onTrackbar, static_cast<void*>(&img));
    setTrackbarMin("sauvola n", "seg contrast stretch", 1);
    createTrackbar("closing", "seg contrast stretch", &c, 30, onTrackbar, static_cast<void*>(&img));
    setTrackbarMin("closing", "seg contrast stretch", 1);
    createTrackbar("connected domains", "seg contrast stretch", &d, 30, onTrackbar, static_cast<void*>(&img));
    setTrackbarMin("connected domains", "seg contrast stretch", 1);

    waitKey(0);
    destroyAllWindows();
    return 0;
}