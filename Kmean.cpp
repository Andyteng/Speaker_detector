#include "opencv2/highgui.hpp"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <string>
#include <algorithm>
#include "opencv2/highgui/highgui.hpp"  
#include "opencv2/imgproc/imgproc.hpp"  

using namespace cv;
using namespace std;

Mat getData (string filName);
// static void help()
// {
//     cout << "\nThis program demonstrates kmeans clustering.\n"
//             "It generates an image with random points, then assigns a random number of cluster\n"
//             "centers and uses kmeans to move those cluster centers to their representitive location\n"
//             "Call\n"
//             "./kmeans\n" << endl;
// }

string DAT_path = "/Users/Andyteng/Dropbox/Graduation_Project/MatchMakersStudentEdition/annotations/new_DATA.csv";
Mat new_DATA = getData(DAT_path);
int co[] = {2, 7, 14, 19, 23, 25, 26, 28};

int main( int, char** argv )
{
    const int MAX_CLUSTERS = 9;
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
    Mat org = imread( argv[1], 1 );  
    Mat img = org.clone();
    RNG rng(12345);
    int frameNum = 0;
    for(;;)
    {
        int k, clusterCount = rng.uniform(2, MAX_CLUSTERS+1);
        int i, sampleCount = rng.uniform(1, 3001);
        Mat points(sampleCount, 1, CV_32FC2), labels;
        clusterCount = MIN(clusterCount, sampleCount);
        Mat centers;
        /* generate random sample from multigaussian distribution */
        cout << clusterCount << endl;
        for( k = 0; k < clusterCount; k++ )
        {
            int sub = co[k];
            cout << co[k] << endl;
            Point center;
            Point originalPoint = Point(new_DATA.at<int>(frameNum,sub*7+1),new_DATA.at<int>(frameNum,sub*7+2));
            Point processPoint = Point(new_DATA.at<int>(frameNum,sub*7+3),new_DATA.at<int>(frameNum,sub*7+4));
            rectangle(img,originalPoint,processPoint,Scalar(255,0,0),2);
            center.x = (originalPoint.x+processPoint.x)/2;
            center.y = (originalPoint.y+processPoint.y)/2;
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
        char key = (char)waitKey();
        if( key == 27 || key == 'q' || key == 'Q' ) // 'ESC'
            break;
    }
    return 0;
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