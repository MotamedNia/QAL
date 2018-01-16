#include <stdio.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <stack>

using namespace std;
using namespace cv;
#define window "QA"
Point pt;
Point prevPt;
bool state;
bool drawing = false;
Mat_<int> mask;

stack<Mat> prevMasks;
stack<Mat> prevImgs;

Mat prevMask;
Mat prevImg;

enum States{GOOD,ACCEPTABLE,BAD,NOT_ACCEPTABLE,EXIT};
const Scalar BLUE = Scalar(0,0,255);
const Scalar BLUE_G = Scalar(200);

const Scalar YELLOW = Scalar(255,255,0);
const Scalar YELLOW_G = Scalar(150);

const Scalar ORANGE = Scalar(200,140,0);
const Scalar ORANGE_G = Scalar(100);

const Scalar RED = Scalar(255,0,50);
const Scalar RED_G = Scalar(50);

Scalar image_color;
Scalar mask_color;

void mouseListener(int event, int x, int y, int flags, void*userdata){
    Mat &img = *((Mat*)(userdata)); // 1st cast it back, then deref
    if(event == EVENT_LBUTTONDOWN){

        prevMasks.push(mask.clone());
        prevImgs.push(img.clone());

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

        prevMasks.push(mask.clone());
        prevImgs.push(img.clone());

        mask[y][x] = mask_color[0];
        img.at<cv::Vec3b>(y,x)[0] = image_color[2];
        img.at<cv::Vec3b>(y,x)[1] = image_color[1];
        img.at<cv::Vec3b>(y,x)[2] = image_color[0];

        prevMask = mask.clone();
        prevImg = img.clone();

        int rows = mask.rows;
        int cols = mask.cols;

        bool flag = true;
        int offset = 1;
        while (flag) {
            flag = false;
            for(int i = (y-offset); i < (y+offset+1);i++) {
                for(int j = (x-offset); j < (x+offset+1);j++) {
                    if (i >= 0 && i < rows - 1 && j >= 0 && j < cols - 1 && mask[i][j] == 0) {
                        if (mask[i - 1][ j] == mask_color[0] || mask[i + 1][ j] == mask_color[0] ||
                                mask[i][ j - 1] == mask_color[0] || mask[i][ j + 1] == mask_color[0]) {
                            mask[i][ j] = mask_color[0];
                            img.at<cv::Vec3b>(i,j)[0] = image_color[2];
                            img.at<cv::Vec3b>(i,j)[1] = image_color[1];
                            img.at<cv::Vec3b>(i,j)[2] = image_color[0];
                            flag = true;
                        }
                    }
                }
            }
            offset++;
        }
    }
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
    image_color = BLUE;
    mask_color = BLUE_G;
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
    resizeWindow(window,720,640);
    setMouseCallback(window,mouseListener,&image);

    States state;

    // initialize prev mats
    prevMask = mask.clone();
    image = image.clone();

    prevMasks.push(mask.clone());
    prevImgs.push(image.clone());

    while (true)
    {
        imshow(window, image);

        //showResults(image);
        char current_state = (char)waitKey(1);

        switch(current_state){
            case 'a'://Good
                image_color = BLUE;
                mask_color = BLUE_G;
                break;
            case 's'://Acceptable
                image_color = YELLOW;
                mask_color = YELLOW_G;
                break;
            case 'd'://Bad
                image_color = ORANGE;
                mask_color = ORANGE_G;
                break;
            case 'f'://NotAcceptable
                image_color = RED;
                mask_color = RED_G;
                break;
            case 'z':
                if (prevImgs.size() > 0) {
                    mask = prevMasks.top();
                    image = prevImgs.top();
                    prevMasks.pop();
                    prevImgs.pop();
                }
                break;
            case 'q':
                state = States::EXIT;
                break;
        }

        if(state == States::EXIT)
            break;
    }

    imwrite("../mask.png",mask);
    imwrite("../img.png",image);
    return 0;
}

