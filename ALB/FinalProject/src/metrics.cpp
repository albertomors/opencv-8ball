/* /*
AUTHOR: Morselli Alberto 
FILE: metrics.cpp

-called by videoHandler
-compute metrics, show and save them
*/

#include "metrics.h"

//r1 = predicted_bb, r2=groundtruth_bb or viceversa
double compute_IoU(const cv::Rect& r1, const cv::Rect& r2){
    cv::Rect I_rect = r1 & r2;
    int I_area = I_rect.area();
    int U_area = r1.area() + r2.area() - I_area;
    double IoU = static_cast<float>(I_area)/static_cast<float>(U_area);
    return IoU;
}

/*
both pred can have less rows than true if we have missed predictions, more rows if we have FP
both have 5 <uint16_t> items per row = x y w h class
*/
std::vector<cv::Point2f> get_PR_table(const cv::Mat& pred_bb, const cv::Mat& true_bb, int pred_class){
    //initialize cumulative indexes
    int TP = 0;
    int FP = 0;
    float P, R;
    std::vector<cv::Point2f> PR_points;

    //pre-iteration over groundtruth to get total GT
    int totalGT = 0;
    for(int i=0; i<pred_bb.rows; ++i)
        totalGT += (true_bb.at<uint16_t>(i,4) == pred_class);

    //handle the scenario where no true_bb is present for the class = all pred are FP
    //there's no possibility you can find it - avoid all checks
    if(totalGT == 0){
        cv::Point2f point(0.0,0.0);
        PR_points.push_back(point); //append point to array
        return PR_points;
    }

    //iterate over predictions
    uint16_t x,y,w,h;
    for(int i=0; i<pred_bb.rows; ++i){
        //but look only at the ones belonging to the class we're analyzing
        if(pred_bb.at<uint16_t>(i,4) == pred_class){ 
            bool isTP = false;

            x = pred_bb.at<uint16_t>(i,0);
            y = pred_bb.at<uint16_t>(i,1);
            w = pred_bb.at<uint16_t>(i,2);
            h = pred_bb.at<uint16_t>(i,3);
            cv::Rect pred_rect(x,y,w,h);
            
            //iterate all over groudtruths
            for(int j=0; j<true_bb.rows; ++j){
                //but look only at the ones belonging to the same class
                if(true_bb.at<uint16_t>(j,4) == pred_class){ 
                    x = true_bb.at<uint16_t>(j,0);
                    y = true_bb.at<uint16_t>(j,1);
                    w = true_bb.at<uint16_t>(j,2);
                    h = true_bb.at<uint16_t>(j,3);
                    cv::Rect true_rect(x,y,w,h);

                    float IoU = compute_IoU(pred_rect, true_rect);
                    //std::cout << i << "," << j << "] " << pred_rect << "vs" << true_rect << " = " << IoU << std::endl;
                    
                    if( IoU >= 0.5){
                        //found the matching => true positive - stop searching
                        isTP = true;
                        break;
                    }
                    //otherwise continue looking for the potential matching
                }
            }

            if(isTP){TP++;}
            else{FP++;}

            //compute P,R indexes
            P = static_cast<float>(TP)/static_cast<float>(TP+FP);
            R = static_cast<float>(TP)/static_cast<float>(totalGT); //true_bb.rows = total groundtruth
            float R_quantized = std::round(R * 10.0f) / 10.0f; //round the R values to +-0.1 for 11-point interpolation

            //std::cout << TP << ":" << FP << " | PR=" << P << ":" << R << "-->" << R_quantized << std::endl;
            
            cv::Point2f point(R_quantized,P); //R=x, P=y
            PR_points.push_back(point); //append point to array
        }
    }

    //handle the scenario where no prediction bb was present => all pred are FN
    if(PR_points.size() == 0){
        cv::Point2f point(0.0,0.0);
        PR_points.push_back(point); //append point to array
        return PR_points;
    }

    //otherwise
    return PR_points;
}

//remove coincident points (deleting the lowest) also flattening oblique segments to the next max.
std::vector<cv::Point2f> refine_PR_table(std::vector<cv::Point2f>& points){

    //keep non overlapping x-points
    std::vector<cv::Point2f> unique_points;
    float last_seen = points[0].x;
    unique_points.push_back(points[0]);

    for(int i=1; i<points.size(); ++i){
        if(points[i].x == last_seen){ //TODO check sull'equality che forse va messa la tolleranza
            //since only v[i+1] <= v[i] can happen | keep the max == [i], remove [i+1]
            continue; //dont push_back anything
        }
        //otherwise push it and update last seen
        unique_points.push_back(points[i]);
        last_seen = points[i].x;
    }

    //now flat the y-values from right to left
    for(int i=unique_points.size()-1; i>0; --i){
        unique_points[i-1].y = std::max(unique_points[i-1].y, unique_points[i].y);
    }

    return unique_points;
}

double compute_mAP(const cv::Mat& pred_bb, const cv::Mat& true_bb){
    double mAP = 0.0;
    const int num_classes = 4;

    //iterate over all classes
    for(int i=1; i<=num_classes; ++i){
        std::vector<cv::Point2f> PR_points = get_PR_table(pred_bb, true_bb, i);
        PR_points = refine_PR_table(PR_points);

        //AP = 1/11 * first + 1/11 * sum(points)
        double AP = 1.0/11 * PR_points[0].y + 10.0/11 * PR_points[0].x * PR_points[0].y;
        for(int j=1; j<PR_points.size(); j++){
            AP += 10.0/11 * (PR_points[j].x - PR_points[j-1].x) * PR_points[j].y; //dx*y
        }
        /*
        the expression is equivalent to summing 11-values weighted by 1/11 [first sample is inclusive]
        1/11 * 2 * y = 1/11 * 10 * 0.2 * y = 10/11 * 0.2 * y
        */

        std::cout << "AP for class " << i << " = " << AP << std::endl; //TODO just for debug

        mAP += AP;
    }

    mAP/=num_classes;
    return mAP;
}

void compute_mIoU(cv::Mat& seg_mask){
    //TODO
    return;
}