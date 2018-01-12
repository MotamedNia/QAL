#include <stdio.h>
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;
#define window "QA"
Point pt;
Point prevPt;
bool state;
bool drawing = false;
Mat_<int> mask;

void mouseListener(int event, int x, int y, int flags, void*userdata){
    Mat &img = *((Mat*)(userdata)); // 1st cast it back, then deref
    if(event == EVENT_LBUTTONDOWN){
        drawing = true;
        prevPt = Point(x,y);
        }
    if(event == EVENT_MOUSEMOVE){
        if(drawing) {
            pt = Point(x, y);
            line(img, pt, prevPt, Scalar(0, 255, 0), 2);
            line(mask,pt,prevPt,Scalar(255));
            prevPt = pt;
        }
    }
    if(event == EVENT_LBUTTONUP){
        if(drawing) {
            pt = Point(x, y);
            line(img, pt, prevPt, Scalar(0, 255, 0), 2);
            drawing = false;
        }
    }

    if(event == EVENT_RBUTTONDOWN){
        mask[y][x] = 124;
        img.at<cv::Vec3b>(y,x)[2] = 255;


        int rows = mask.rows;
        int cols = mask.cols;

        bool flag = true;
        int offset = 1;
        while (flag) {
            flag = false;
            for(int i = (y-offset); i < (y+offset+1);i++) {
                for(int j = (x-offset); j < (x+offset+1);j++) {
                    if (i >= 0 && i < rows - 1 && j >= 0 && j < cols - 1 && mask[i][j] == 0) {
                        if (mask[i - 1][ j] == 124 || mask[i + 1][ j] == 124 || mask[i][ j - 1] == 124 || mask[i][ j + 1] == 124) {
                            mask[i][ j] = 124;
                            img.at<cv::Vec3b>(i,j)[2] = 255;
                            flag = true;
                        }
                    }
                }
            }
            offset++;
        }
    }
}

void showResults(Mat img){
    Point pt2 = pt;
    if(state)
        cout<< "Hi";
    else
    if(!(pt2.x == prevPt.x && pt2.y == prevPt.y))
        cout<< pt2;
    prevPt = Point(pt.x,pt.y);
}


int main(int argc, char** argv )
{
    if ( argc != 2 )
    {
        printf("usage: DisplayImage.out <Image_Path>\n");
        return -1;
    }

    // Variables
    Mat image;

    image = imread( argv[1], 1 );
    //image = imread("../test3.jpg");

    if ( !image.data )
    {
        printf("No image data \n");
        return -1;
    }

    // initialize mask
    mask = Mat_<int>::zeros(image.rows,image.cols);

    namedWindow(window, WINDOW_NORMAL );
    resizeWindow(window,1280,720);
    setMouseCallback(window,mouseListener,&image);
    while (true)
    {
        imshow(window, image);

        //showResults(image);
        int k = waitKey(1);
        if (k == 'q')
            break;
    }

    imwrite("../mask.png",mask);
    imwrite("../img.png",image);
    return 0;
}

