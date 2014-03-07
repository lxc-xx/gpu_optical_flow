#include "opencv2/video/tracking.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/gpu/gpu.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include "opencv2/nonfree/gpu.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <sstream>

using namespace std;
using namespace cv;

#define KEY_SPACE (1048608)
#define KEY_RIGHT (1113939)
#define KEY_LEFT (1113937)
#define KEY_ESC (1048603)

Mat** now_gaussian_octaves;
Mat** next_gaussian_octaves;

#define STATE_PAUSE (0)
#define STATE_END (1)
#define STATE_SINGLE_STEP (2)
#define STATE_NORMAL (3)

static void drawOptFlowMap(const Mat& flow, Mat& cflowmap, int step,
                    double, const Scalar& color)
{
    for(int y = 0; y < cflowmap.rows; y += step)
        for(int x = 0; x < cflowmap.cols; x += step)
        {
            const Point2f& fxy = flow.at<Point2f>(y, x);
            line(cflowmap, Point(x,y), Point(cvRound(x+fxy.x), cvRound(y+fxy.y)),
                 color);
            circle(cflowmap, Point(x,y), 2, color, -1);
        }
}


int main()
{
    //Mat img = imread("data/steve-jobs.jpg", CV_LOAD_IMAGE_GRAYSCALE);
    //Mat img = imread("data/Lenna.png", CV_LOAD_IMAGE_GRAYSCALE);

    //VideoCapture capture("data/sandwich1.mp4");
    VideoCapture capture("data/handwaving.avi");
    //VideoCapture capture("data/handclapping.avi");
    //VideoCapture capture("data/jogging.avi");
    //VideoCapture capture("/home/lxc/Code/sed/video/Dev08-1/LGW_20071101_E1_CAM1.avi");
    //VideoCapture capture("/home/lxc/Code/Mosift_Compare/HVC003163.mp4");
    //VideoCapture capture("/home/lxc/Downloads/big_buck.avi");

    if(!capture.isOpened())
    {
        cout << "Cannot open the video file" << endl;
        return -1;
    }

    namedWindow("video", CV_WINDOW_AUTOSIZE);
    Mat img;
    Mat frame;
    Mat prev;
    bool first_frame = true;

    while(true)
    {
        if(!capture.read(frame))
        {
            cout << "Cannot read frame" << endl;
            break;
        }
        cvtColor(frame, img, CV_BGR2GRAY); 

        if( first_frame )
        {
            first_frame = false;
            img.copyTo(prev);
            continue;
        }
        imshow("video", img);

        /*
        Mat flow, cflow;
        calcOpticalFlowFarneback(prev, img, flow, 0.5, 3, 15, 3, 5,     1.2, 0);
        cvtColor(prev, cflow, CV_GRAY2BGR);
        drawOptFlowMap(flow, cflow, 16, 1.5, CV_RGB(0, 255, 0));
        imshow("flow", cflow);
        */
        

        gpu::GpuMat gPrv(prev);
        gpu::GpuMat gNxt(img);
        gpu::GpuMat gFlwX;
        Mat FlwX;
        gpu::GpuMat gFlwY;
        Mat FlwY;
        gpu::FarnebackOpticalFlow farnebackOpticalFlow;
        farnebackOpticalFlow(gPrv, gNxt, gFlwX, gFlwY);
        gFlwX.download(FlwX);
        gFlwY.download(FlwY);

        
        //extraxt x and y channels
        Mat xy[2]; //X,Y
        xy[0] = FlwX;
        xy[1] = FlwY;
        
        //merge( xy, 2, flow_pyr[k]);

        //calculate angle and magnitude
        Mat magnitude, angle;
        cartToPolar(xy[0], xy[1], magnitude, angle, true);

        //translate magnitude to range [0;1]
        double mag_max;
        double mag_min;
        minMaxLoc(magnitude, &mag_min, &mag_max);
        magnitude.convertTo(magnitude, -1, 1.0/mag_max);

        //build hsv image
        Mat _hsv[3], hsv;
        _hsv[0] = angle;
        _hsv[1] = cv::Mat::ones(angle.size(), CV_32F);
        _hsv[2] = magnitude;
        merge(_hsv, 3, hsv);

        //convert to BGR and show
        Mat bgr;//CV_32FC3 matrix
        cvtColor(hsv, bgr, cv::COLOR_HSV2BGR);
        imshow("optical flow", bgr);

        int key = waitKey(2);
            
        img.copyTo(prev);
    }

    return 0;
}
