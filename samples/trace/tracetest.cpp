#include <stdio.h>
#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>

using namespace cv;

int main(int, char**)
{
    //VideoCapture cap("/home/root/opencv/Megamind.avi"); // open the file
    VideoCapture cap("Megamind.avi"); // open the file
    if(!cap.isOpened())  // check if we succeeded
    {
        printf("Failed to open file /home/root/opencv/Megamind.avi\n");
        return -1;
    }
    printf("Opened file home/root/opencv/Megamind.avi\n");

    /*Mat edges;
    namedWindow("edges",1);
    for(;;)
    {
        Mat frame;
        cap >> frame; // get a new frame from camera
        cvtColor(frame, edges, CV_BGR2GRAY);
        GaussianBlur(edges, edges, Size(7,7), 1.5, 1.5);
        Canny(edges, edges, 0, 30, 3);
        imshow("edges", edges);
        if(waitKey(30) >= 0) break;
    } */

    cap.release();

    return 0;
}

