#include <stdio.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <stack>

using namespace std;
using namespace cv;
#define window "QA"
Point pt;
Point prevPt;
bool drawing = false;
Mat_<int> mask;

stack<Mat> prevMasks;
stack<Mat> prevImgs;

Mat prevMask;
Mat prevImg;

enum States{Special_Corners,Draw_Corners,Labeling,EXIT};

States state;

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

int thickness;

Point cornerValues[4];

const String corners[] ={"TL_corner","TR_corner","BR_corner","BL_corner",};
int CI = 0;//Corner index

void snale(Mat &img,int x,int y){
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

void recDraw(Mat &img,int x,int y,int &rows,int &cols){
    if (y >= 0 && y < rows - 1 && x >= 0 && x < cols - 1 && mask[y][x] == 0){

        mask[y][x] = mask_color[0];
        img.at<cv::Vec3b>(y,x)[0] = image_color[2];
        img.at<cv::Vec3b>(y,x)[1] = image_color[1];
        img.at<cv::Vec3b>(y,x)[2] = image_color[0];

        stack<Point> poitns;
        poitns.push(Point(x-1,y));
        poitns.push(Point(x,y-1));
        poitns.push(Point(x+1,y));
        poitns.push(Point(x,y+1));

        while (!poitns.empty()){
            Point point = poitns.top();
            poitns.pop();
            if (point.y >= 0 && point.y < rows - 1 && point.x >= 0 && point.x < cols - 1 && mask[point.y][point.x] == 0){
                mask[point.y][point.x] = mask_color[0];
                img.at<cv::Vec3b>(point.y,point.x)[0] = image_color[2];
                img.at<cv::Vec3b>(point.y,point.x)[1] = image_color[1];
                img.at<cv::Vec3b>(point.y,point.x)[2] = image_color[0];

                poitns.push(Point(point.x-1,point.y));
                poitns.push(Point(point.x,point.y-1));
                poitns.push(Point(point.x+1,point.y));
                poitns.push(Point(point.x,point.y+1));
            }
        }
    }

}

void mouseListener(int event, int x, int y, int flags, void*userdata){
    Mat &img = *((Mat*)(userdata)); // 1st cast it back, then deref
    if(event == EVENT_LBUTTONDOWN){

        switch (state){
            case Labeling:
                prevMasks.push(mask.clone());
                prevImgs.push(img.clone());

                drawing = true;
                prevPt = Point(x,y);
                break;
            case Special_Corners:
                prevMasks.push(mask.clone());
                prevImgs.push(img.clone());

                if (CI < 4) {
                    cornerValues[CI] = Point(x, y);
                    putText(img, corners[CI], cornerValues[CI], FONT_HERSHEY_PLAIN, thickness/2, Scalar(0, 120, 120), thickness);
                    CI++;
                    if (CI==4)
                        state = States ::Draw_Corners;
                }
                break;

            case Draw_Corners:
                for(int i =0;i<4;i++){
                    if (i <3) {
                        line(img, cornerValues[i], cornerValues[i + 1], Scalar(120, 120, 0));
                        line(mask, cornerValues[i], cornerValues[i + 1], Scalar(120));
                    }
                    else {
                        line(img, cornerValues[i], cornerValues[0], Scalar(120, 120, 0));
                        line(mask, cornerValues[i], cornerValues[0], Scalar(120));
                    }
                }
                break;
        }

        }
    if(event == EVENT_MOUSEMOVE){
        if(drawing) {
            pt = Point(x, y);
            line(img, pt, prevPt, Scalar(0, 255, 0), thickness);
            line(mask,pt,prevPt,Scalar(255));
            prevPt = pt;
        }
    }
    if(event == EVENT_LBUTTONUP){
        if(drawing) {
            pt = Point(x, y);
            line(img, pt, prevPt, Scalar(0, 255, 0), thickness);
            line(mask,pt,prevPt,Scalar(255));
            drawing = false;
        }
    }

    if(event == EVENT_RBUTTONDOWN){
        prevMasks.push(mask.clone());
        prevImgs.push(img.clone());
        try {
            int rows = mask.rows;
            int cols = mask.cols;
            //snale(img, x,y);
            recDraw(img,x,y,rows,cols);
        }
        catch (Exception e){
            cout << e.err;
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

    int num = image.rows;
    int dnum = 720;

    if(num < image.cols) {
        num = image.cols;
        dnum = 640;
    }
    thickness = (int)(num/dnum)*2;

    namedWindow(window, WINDOW_NORMAL );
    resizeWindow(window,720,640);
    setMouseCallback(window,mouseListener,&image);

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

                    if (state == Special_Corners && CI > 0) {
                        CI--;
                    }
                    else if (state == Draw_Corners && CI > 0) {
                        CI--;
                        state = Special_Corners;
                    }

                }
                break;
            case 'q':
                state = States::EXIT;
                break;
            case 'w':
                state = States::Labeling;
                break;
            case 'e':
                state = States::Special_Corners;
                break;
            default:
                break;
        }

        if(state == States::EXIT)
            break;
    }


    imwrite("../mask.png",mask);
    imwrite("../img.png",image);
    return 0;
}

