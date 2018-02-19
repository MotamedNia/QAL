#include <fstream>
#include <stdio.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <stack>
#include <sys/stat.h>
#include <math.h>
#include "json.hpp"

using json = nlohmann::json;
using namespace std;
using namespace cv;

#define window "QA"


Point pt;
Point prevPt;
bool drawing;
Mat_<int> mask;

stack<Mat> prevMasks;
stack<Mat> prevImgs;

Mat nextMask;
Mat nextImg;

enum States{Special_Corners,Draw_Corners,Labeling,ZOOM,EXIT, SKIP
};

States state;

const Scalar BLUE = Scalar(0,0,255);
const Scalar BLUE_G = Scalar(200);

const Scalar YELLOW = Scalar(255,255,0);
const Scalar YELLOW_G = Scalar(150);

const Scalar ORANGE = Scalar(200,140,0);
const Scalar ORANGE_G = Scalar(100);

const Scalar RED = Scalar(255,0,50);
const Scalar RED_G = Scalar(50);

const Scalar paperColor = Scalar(125,125,0);
const Scalar paperColor_G = Scalar(125);

const Scalar regionColor = Scalar(0,255,0);
const Scalar regionColor_G = Scalar(255);

Scalar image_color;
Scalar mask_color;

int thickness;

Point* cornerValues;

const String corners[] ={"TL","TR","BR","BL",};
int CI = 0;//Corner index
int II = 0;//Image index


void makeupMask();

void restVars() {
    CI =0;
    state = States ::Special_Corners;
    drawing = false;
    prevImgs = stack<Mat>();
    prevMasks = stack<Mat>();
    cornerValues = new Point[4];
}

void snale(Mat &img, int x, int y){
    mask[y][x] = mask_color[0];
    img.at<cv::Vec3b>(y,x)[0] = image_color[2];
    img.at<cv::Vec3b>(y,x)[1] = image_color[1];
    img.at<cv::Vec3b>(y,x)[2] = image_color[0];


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
            case ZOOM://TODO has many bugs
                prevMasks.push(mask.clone());
                prevImgs.push(img.clone());
                img = img(Rect(x-200,y-200,400,400));
                break;
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
                    putText(img, corners[CI], cornerValues[CI], FONT_HERSHEY_SIMPLEX, thickness/2.9, Scalar(0, 100, 255), thickness);
                    CI++;
                    if (CI==4)
                        state = States ::Draw_Corners;
                }
                break;

            case Draw_Corners:
                for(int i =0;i<4;i++){
                    if (i <3) {
                        line(img, cornerValues[i], cornerValues[i + 1], paperColor,thickness);
                        line(mask, cornerValues[i], cornerValues[i + 1], paperColor_G);
                    }
                    else {
                        line(img, cornerValues[i], cornerValues[0], paperColor,thickness);
                        line(mask, cornerValues[i], cornerValues[0], paperColor_G);
                    }
                }
                break;
        }

        }
    if(event == EVENT_MOUSEMOVE){
        if(drawing) {
            pt = Point(x, y);
            line(img, pt, prevPt, regionColor, thickness);
            line(mask,pt,prevPt,regionColor_G);
            prevPt = pt;
        }
    }
    if(event == EVENT_LBUTTONUP){
        if(drawing) {
            pt = Point(x, y);
            line(img, pt, prevPt, regionColor, thickness);
            line(mask,pt,prevPt,regionColor_G);
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
    mkdir("outputs",S_IRWXU|S_IRWXG|S_IRWXO);
    if ( argc != 2 )
    {
        printf("usage: ./CVersion <Video_Path>\n");
        return -1;
    }

    // Variables
    Mat refImage;
    Mat image;
    image_color = BLUE;
    mask_color = BLUE_G;

        //Reading Video file
    String filePath = argv[1];
    VideoCapture capture = VideoCapture(filePath);

    while (capture.read(image)) {

        restVars();
        refImage.release();

        //End process when there is no frame
        if (!image.data) {
            printf("No image data \n");
            break;
        }

        // initialize mask
        mask = Mat_<int>::zeros(image.rows, image.cols);

        int num = image.rows;
        int dnum = 720;

        if (num < image.cols) {
            num = image.cols;
            dnum = 640;
        }

        thickness = (int) (num / dnum) * 2;

        namedWindow(window, WINDOW_NORMAL);
        resizeWindow(window, 1366, 768);
        setMouseCallback(window, mouseListener, &image);

        prevMasks.push(mask.clone());
        prevImgs.push(image.clone());

        //Increment image index(II)
        II++;
        putText(image,to_string(II),Point(40,40),FONT_HERSHEY_PLAIN,thickness/2.5,Scalar(0,100,255),thickness);
        refImage = image.clone();

        while (true) {
            imshow(window, image);

            //showResults(image);
            char current_state = (char) waitKey(1);

            switch (current_state) {
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
                        nextImg = image;
                        nextMask = mask;
                        mask = prevMasks.top();
                        image = prevImgs.top();
                        prevMasks.pop();
                        prevImgs.pop();

                        if (state == Special_Corners && CI > 0) {
                            CI--;
                        } else if (state == Draw_Corners && CI > 0) {
                            CI--;
                            state = Special_Corners;
                        }

                    }
                    break;
                case 'y'://TODO is not complete
                    if (!nextMask.empty() && !nextImg.empty()) {
                        image = nextImg;
                        mask = nextMask;
                        if (state == Special_Corners && CI < 4) {
                            CI++;
                            if (CI == 4)
                                state = Draw_Corners;
                        }
                    }
                    break;
                case 'q':
                    state = States::EXIT;
                    break;
                case 'w':
                    state = States::Labeling;
                    prevImgs = stack<Mat>();
                    prevMasks = stack<Mat>();
                    break;
                case 'e':
                    state = States::Special_Corners;
                    break;
                case 'r':
                    state = States::ZOOM;
                    break;
                case 'x':
                    state = States::SKIP;
                    break;
                default:
                    break;
            }

            if (state == States::EXIT || state == States::SKIP)
                break;
        }



        if (state != States::SKIP) {
            json data = {
                    {"mask color Information", {
                                                       {"GOOD",     BLUE_G[0]},
                                                       {"ACCEPTABLE", YELLOW_G[0]},
                                                       {"BAD",         ORANGE_G[0]},
                                                       {"NOT ACCEPTABLE", RED_G[0]},
                                                       {"PAPER COLOR", paperColor_G[0]},
                                                       {"REGION COLOR", regionColor_G[0]}
                                               }},
                    {"paper coordination",     {
                                                       {"TOP LEFT", {{"X", cornerValues[0].x}, {"Y", cornerValues[0].y}}},
                                                       {"TOP RIGHT",  {{"X", cornerValues[1].x}, {"Y", cornerValues[1].y}}},
                                                       {"BOTTOM LEFT", {{"X", cornerValues[2].x}, {"Y", cornerValues[2].y}}},
                                                       {"BOTTOM RIGHT",   {{"X", cornerValues[3].x}, {"Y", cornerValues[3].y}}}
                                               }

                    }
            };

            std::ofstream o("outputs/"+to_string(II)+"data.json");
            o << std::setw(4) << data << std::endl;

            makeupMask();
            String imageFile = "outputs/"+to_string(II)+"img.png";
            String maskFile = "outputs/"+to_string(II)+"mask.png";
            imwrite(imageFile, image);
            imwrite(maskFile, mask);

            String refImageFile = "outputs/"+to_string(II)+"refImg.png";
            imwrite(refImageFile, refImage);
        }
        if (state == States::SKIP){
//            String imageFile = "outputs/"+to_string(II)+"refImg.png";
//            imwrite(imageFile, image);
        }

    }
    return 0;
}

void makeupMask() {

    Mat temp = Mat::zeros(mask.size(),mask.type());

    for (int i=0; i< mask.rows ; i++){
        for (int j = 0; j < mask.cols; ++j) {
            int l = 0, t = 0, b = 0, r = 0;

            if (mask.at<int>(i, j) == 125 || mask.at<int>(i, j) == 255) {
                if (i - 1 >= 0 && mask.at<int>(i-1, j) != 125 && mask.at<int>(i-1, j) != 255)
                    l = mask.at<int>(i-1, j);
                if (i + 1 >= 0 && mask.at<int>(i+1, j) != 125 && mask.at<int>(i+1, j)!= 255)
                    r = mask.at<int>(i+1, j);
                if (j - 1 >= 0 && mask.at<int>(i, j-1) != 125 && mask.at<int>(i, j-1) != 255)
                    b = mask.at<int>(i, j-1);
                if (j - 1 >= 0 && mask.at<int>(i, j+1) != 125 && mask.at<int>(i, j+1) != 255)
                    t = mask.at<int>(i, j+1);

                int max1 = max(l,r);
                int max2 = max(t,b);
                temp.at<int>(i,j) = max(max1,max2);
            } else
                temp.at<int>(i,j) =mask.at<int>(i,j);
        }
    }

    mask = temp.clone();
    temp.release();

}


