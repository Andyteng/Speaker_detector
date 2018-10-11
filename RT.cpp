#include <fstream>
#include <vector>
#include <iterator>
#include <string>
#include <algorithm>
#include "opencv2/core/core.hpp"  
#include "opencv2/highgui/highgui.hpp"  
#include "opencv2/imgproc/imgproc.hpp"  
#include<iostream>  

using namespace cv;  
using namespace std;  

Mat getData (string filName);
Mat getDST (int frameNum,int sub);
vector<int> getID(int frameNum, int camID);
vector<int> subs;
Mat image;
Mat imageCopy; //绘制矩形框时用来拷贝原图的图像
int subNum = 92;
int fps = 20;
int sub;
Point originalPoint ; //矩形框起点
Point processPoint ; //矩形框终点
string DAT_path = "/Users/Andyteng/Dropbox/Graduation_Project/MatchMakersStudentEdition/annotations/new_DATA.csv";
string CAM_path = "/Users/Andyteng/Dropbox/Graduation_Project/MatchMakersStudentEdition/annotations/new_CAMERA.csv";
string PAR_path = "/Users/Andyteng/Dropbox/Graduation_Project/MatchMakersStudentEdition/annotations/new_PARTICIPANTS.csv";
string DRINK_path = "/Users/Andyteng/Dropbox/Graduation_Project/MatchMakersStudentEdition/annotations/partitions/drinking/drinking_DAY3_CAM12.csv";
Mat new_DATA = getData(DAT_path);
Mat new_CAMERA = getData(CAM_path);
Mat new_PAR = getData(PAR_path);
Mat new_DRINK = getData(DRINK_path);

int main(int argc,char*argv[])  
{  
    int frameNum = 0;
	VideoCapture video(argv[1]);
	double fps=video.get(CV_CAP_PROP_FPS); //获取视频帧率
	double pauseTime=1000/fps; //两幅画面中间间隔
	namedWindow("Video");


    int start_frame_no = 0; // convert our input string to number - C++ style
    int end_frame_no = 0;

    if (!video.isOpened())
    {
        cout  << "Could not open the input video: " << argv[1] << endl;
        return -1;
    }

    subs = getID(frameNum,2);

        while(true)
        {
        video>>image;

        if(!image.data||waitKey(pauseTime)==27 || frameNum >video.get(CAP_PROP_FRAME_COUNT))  //图像为空或Esc键按下退出播放
        {
            break;
        }
        
        if (new_CAMERA.at<int>(frameNum,subs[0]) == 2) //Day 1: 2 -> CAM13
        {
            Mat dst = getDST(frameNum,subs[0]);
        imshow("Video",image);	
        frameNum++;	
	    }
	return 0;  
        }


	

// write video
Mat getDST (int frameNum, int sub)
{
     Mat dst;
    Size size(140,130);

    originalPoint = Point(new_DATA.at<int>(frameNum,sub*7+1),new_DATA.at<int>(frameNum,sub*7+2));
    processPoint = Point(new_DATA.at<int>(frameNum,sub*7+3),new_DATA.at<int>(frameNum,sub*7+4));
    rectangle(image,originalPoint,processPoint,Scalar(255,0,0),2);
    //outputVideo.write(image(Rect(originalPoint,processPoint)));
    
    //cout<< image(Rect(originalPoint,processPoint)).size << "\n";
    resize(image(Rect(originalPoint,processPoint)),dst,size);
    return dst;
}

// Identify subjects under a certain camera
vector<int> getID(int frameNum, int camID)
{
    vector<int> subjects;
    for (int par = 0; par <32; par++)
    {
        if (new_CAMERA.at<int>(frameNum,par) == camID)
        {
            subjects.push_back(par);
        }
    }
    return subjects;
}
//CSVReader
 Mat getData(string fileName)
 {
    ifstream inputfile(fileName);
        string current_line;
    // vector allows you to add data without knowing the exact size beforehand
    vector< vector<int> > all_data;
    // Start reading lines as long as there are lines in the file
    while(getline(inputfile, current_line)){
    // Now inside each line we need to seperate the cols
    vector<int> values;
    stringstream temp(current_line);
    string single_value;
    while(getline(temp,single_value,',')){
            // convert the string element to a integer value
            values.push_back(atoi(single_value.c_str()));
    }
    // add the row to the complete data vector
    all_data.push_back(values);
    }

    // Now add all the data into a Mat element (new_DATA 36000*92*7)
    Mat new_DATA = Mat::zeros((int)all_data.size(), (int)all_data[0].size(), CV_8UC1);
    // Loop over vectors and add the data
    for(int rows = 0; rows < (int)all_data.size(); rows++){
        for(int cols= 0; cols< (int)all_data[0].size(); cols++){
            new_DATA.at<int>(rows,cols) = all_data[rows][cols];
        }
    }
    return new_DATA;
 }


/*
 Mat getKClusters(const int MAX_CLUSTERS, Mat img)
 {
    Scalar colorTab[] =
    {
        Scalar(0, 0, 255),
        Scalar(0,255,0),
        Scalar(255,100,100),
        Scalar(255,0,255),
        Scalar(0,255,255),
        Scalar(0,166,216),
        Scalar(110,187,213),
        Scalar(29,28,115),
        Scalar(230,70,22),
        Scalar(225,196,0),
        Scalar(226,26,26),
        Scalar(0,136,145),
        //Scalar(165,202,26),
        //Scalar(109,23,127),
        //Scalar(107,134,137),

    };
    RNG rng(12345);
    for(;;)
    {
        int k, clusterCount = rng.uniform(2, MAX_CLUSTERS+1);
        int i, sampleCount = rng.uniform(1, 36000);
        Mat points(sampleCount, 1, CV_32FC2), labels;
        clusterCount = MIN(clusterCount, sampleCount);
        Mat centers;
        /* generate random sample from multigaussian distribution 
        for( k = 0; k < clusterCount; k++ )
        {
            Point center;
            center.x = rng.uniform(0, img.cols);
            center.y = rng.uniform(0, img.rows);
            Mat pointChunk = points.rowRange(k*sampleCount/clusterCount,
                                             k == clusterCount - 1 ? sampleCount :
                                             (k+1)*sampleCount/clusterCount);
            rng.fill(pointChunk, RNG::NORMAL, Scalar(center.x, center.y), Scalar(img.cols*0.05, img.rows*0.05));
        }
        randShuffle(points, 1, &rng);
        kmeans(points, clusterCount, labels,
            TermCriteria( TermCriteria::EPS+TermCriteria::COUNT, 10, 1.0),
               3, KMEANS_PP_CENTERS, centers);
        //img = Scalar::all(0);
        for( i = 0; i < sampleCount; i++ )
        {
            int clusterIdx = labels.at<int>(i);
            Point ipt = points.at<Point2f>(i);
            circle( img, ipt, 2, colorTab[clusterIdx], FILLED, LINE_AA );
        }
        imshow("clusters", img);
    return points;

 }
*/