/*
Task 1
Write a program that opens two images (filenames provided as command-line arguments -
files provided on moodle) and checks the content by: i) evaluating features on the images; ii)
matching features of the two images. The matching strategy is at your choice.
Based on the feature match the program shall state whether:
● the two images have similar content;
● the two images have similar content processed by some strong transformation (e.g.,
the content is strongly rotated, or framed under a very different perspective);
● the two images have different content.
Test your approach considering different features available in OpenCV: check on the
documentation, list all the available features, select 2-3 features you consider suitable for the
task. Test on different image couples - start from the images provided, but optionally you can
test on other images taken from the internet or shot by yourself.
*/

#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>

using namespace cv;
using namespace cv::xfeatures2d;
using namespace std;

int main(int argc, char** argv){
    
    //load images
    const string path1 = (argc>1) ? argv[1] : "Images/img1.png";
    const string path2 = (argc>2) ? argv[2] : "Images/img2.png";
    Mat img1 = imread(path1);
    Mat img2 = imread(path2);

    if (img1.empty() || img2.empty()){
        cout << "Invalid filepaths." << endl;
        return -1;
    }

    //SIFT detection and description
    Ptr<Feature2D> detector = ORB::create();
    vector<KeyPoint> keypoints1, keypoints2;
    Mat descriptors1, descriptors2;
    detector->detectAndCompute(img1, noArray(), keypoints1, descriptors1);
    detector->detectAndCompute(img2, noArray(), keypoints2, descriptors2);

    //FLANN matcher
    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::BRUTEFORCE_HAMMING);
    vector<vector<DMatch>> knn_matches;
    matcher->knnMatch(descriptors1, descriptors2, knn_matches, 2);

    //ratio test
    const float ratio_thresh = (argc > 3) ? clamp(stof(argv[3]), 0.0f, 1.0f) : 0.7f;
    vector<DMatch> good_matches;
    for (size_t i = 0; i < knn_matches.size(); i++){
        cout << knn_matches[i][0].distance << "vs" << knn_matches[i][1].distance << endl;
        if (knn_matches[i][0].distance < ratio_thresh * knn_matches[i][1].distance)
            good_matches.push_back(knn_matches[i][0]);
    }

    Mat img_matches;
    drawMatches(img1, keypoints1, img2, keypoints2, good_matches, img_matches, 
                Scalar::all(-1), Scalar::all(-1), std::vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
    namedWindow("knn", WINDOW_NORMAL); imshow("knn", img_matches);

    //similarity between two images
    float similarity = static_cast<double>(good_matches.size())/static_cast<double>(knn_matches.size());
    cout << "num of matches: " << knn_matches.size() << endl;
    cout << "after ratio test: " << good_matches.size() << endl;
    cout << "index of similarity: " << similarity << endl;

    //index of distortion
    vector<Point2f> pos1, pos2;
    for (size_t i = 0; i < good_matches.size(); i++) {
        pos1.push_back(keypoints1[good_matches[i].queryIdx].pt);
        pos2.push_back(keypoints2[good_matches[i].trainIdx].pt);
    }
    
    cout << pos1[0] << endl;
    cout << pos2[0] << endl;
    
    cv::waitKey(0);
    return 0;
}