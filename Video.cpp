#include <opencv/cv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <ctype.h>
#include <unistd.h>

#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <list>
#include <string>

using namespace cv;
using namespace std;


IplImage* image = 0; 
IplImage* prev_image = 0;
CvCapture* capture = 0;
Rect r=Rect(10,20,40,60);
Point p1 = Point(10,20);
Point p2 = Point(40,60);

int show = 1; 

int main( int argc, char** argv )
{
	int frameNum = 0;

	char* video = argv[1];
	capture = cvCreateFileCapture(video);

	if( !capture ) { 
		printf( "Could not initialize capturing..\n" );
		return -1;
	}
	
	if( show == 1 )
		cvNamedWindow( "Video", 0 );

	while( true ) {
		IplImage* frame = 0;
		int i, j, c;

		// get a new frame
		frame = cvQueryFrame( capture );
		if( !frame )
			break;

		if( !image ) {
			image =  cvCreateImage( cvSize(frame->width,frame->height), 8, 3 );
			image->origin = frame->origin;
		}

		cvCopy( frame, image, 0 );

		if( show == 1 ) {
			cvShowImage( "Video", image);
			rectangle(cvarrToMat(image),p1,p2,Scalar(255,0,0),1,8,0);
			c = cvWaitKey(3);
			if((char)c == 27) break;
		}
		if (frameNum <500){
			std::cerr << "The " << frameNum << "-th frame" << std::endl;
			frameNum++;
		}else{
			printf("The Video is destroied \n");
			break;
		}
	}

	if( show == 1 )
		cvDestroyWindow("Video");

	return 0;
}
