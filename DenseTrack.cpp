#include "DenseTrack.h"
#include "Initialize.h"
#include "Descriptors.h"
#include "OpticalFlow.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//to print trajectories in file
#include <iostream>
#include <fstream>
#include <sys/stat.h>

using namespace std;
using namespace cv;

//function declaration
vector<vector<int> > getData(string filename);

string DAT_path = "/home/andy/Dropbox/Graduation_Project/MatchMakersStudentEdition/annotations/partitions/DATA_part1.csv";
string PAR_path = "/home/andy/Dropbox/Graduation_Project/MatchMakersStudentEdition/annotations/partitions/Day1_CAM12_part1.csv";
string DRINK_DAY1_path = "/home/andy/Dropbox/Graduation_Project/MatchMakersStudentEdition/annotations/partitions/drinking/drinking_DAY1.csv";
string DRINK_path = "/home/andy/Dropbox/Graduation_Project/MatchMakersStudentEdition/annotations/partitions/drinking/drinking_DAY3_CAM12.csv";
vector<vector<int> > new_DATA = getData(DAT_path);
vector<vector<int> > new_PAR = getData(PAR_path);
vector<vector<int> > new_DRINK_DAY1 = getData(DRINK_DAY1_path);
vector<vector<int> > new_DRINK = getData(DRINK_path);

//template <typename T> string tostr(const T& t) {
//   ostringstream os;
//   os<<t;
//   return os.str();
//}

int show_track = 1; // set show_track = 1, if you want to visualize the trajectories

vector<int> x1;
vector<int> x2;
vector<int> yy1;
vector<int> y2;

vector<int> x1Drink;
vector<int> x2Drink;
vector<int> y1Drink;
vector<int> y2Drink;

int main(int argc, char **argv)
{
	VideoCapture capture;
	char *video = argv[1];
	char c[100];
	int L = (unsigned)strlen(video);
	strcpy(c, video);

	char *seq = video;

	char comp[4];
	comp[0] = c[L - 3];
	comp[1] = c[L - 2];
	comp[2] = c[L - 1];
	comp[3] = '\0';

	if (strcmp(comp, "mp4") != 0)
	{
		strcat(seq, "%05d.png");
		printf("Loading SEQUENCE in %s\n", video);
	}
	else
	{
		printf("Loading VIDEO %s\n", video);
	}

	//string temppath = "/home/andy/Dropbox/Graduation_Project/MILES_gestures/DT_Segment_30min_Day1_CAM13_20fps_960x5401-500/subject" + tostr(curSub);
	//const char *filename = temppath.c_str();
	char *filename = argv[2];
	int curSub = atoi(argv[3]);
	int flag = arg_parse(argc, argv);
	capture.open(seq);

	if (filename == 0)
	{
		fprintf(stderr, "Output filename not given....\nUsage: DenseTrack video_file trajectories_folder_name [options]\n");
		return -1;
	}

	if (!capture.isOpened())
	{
		fprintf(stderr, "Could not initialize capturing..\n");
		return -1;
	}

	int status;
	status = mkdir(filename, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	int frame_num = 0;
	TrackInfo trackInfo;
	DescInfo hogInfo, hofInfo, mbhInfo;

	InitTrackInfo(&trackInfo, track_length, init_gap);
	InitDescInfo(&hogInfo, 8, false, patch_size, nxy_cell, nt_cell);
	InitDescInfo(&hofInfo, 9, true, patch_size, nxy_cell, nt_cell);
	InitDescInfo(&mbhInfo, 8, false, patch_size, nxy_cell, nt_cell);

	SeqInfo seqInfo;
	InitSeqInfo(&seqInfo, video);

	//end_frame=capture.get(CAP_PROP_FRAME_COUNT);
	//printf("%d in total", end_frame);

	if (flag)
		seqInfo.length = end_frame - start_frame + 1;
	else
		end_frame = seqInfo.length;

	//	fprintf(stderr, "video size, length: %d, width: %d, height: %d\n", seqInfo.length, seqInfo.width, seqInfo.height);

	if (show_track == 1)
		namedWindow("DenseTrack", 0);

	Mat image, prev_grey, grey;

	std::vector<float> fscales(0);
	std::vector<Size> sizes(0);

	std::vector<Mat> prev_grey_pyr(0), grey_pyr(0), flow_pyr(0);
	std::vector<Mat> prev_poly_pyr(0), poly_pyr(0); // for optical flow

	std::vector<std::list<Track> > xyScaleTracks;
	int init_counter = 0; // indicate when to detect new feature points

	//ofstream myfile;
	//myfile.open (filename);
	string str = "";

	while (true)
	{
		Mat frame;
		int i, j, c;
		//re-set string for trajectories store
		str = "";

		// get a new frame
		capture >> frame;
		if (frame.empty())
			break;

		if (frame_num < start_frame || frame_num > end_frame)
		{
			frame_num++;
			continue;
		}
		printf("Processing %d of %d\n", frame_num, end_frame);
		if (frame_num == start_frame)
		{
			image.create(frame.size(), CV_8UC3);
			grey.create(frame.size(), CV_8UC1);
			prev_grey.create(frame.size(), CV_8UC1);

			InitPry(frame, fscales, sizes);

			BuildPry(sizes, CV_8UC1, prev_grey_pyr);
			BuildPry(sizes, CV_8UC1, grey_pyr);

			BuildPry(sizes, CV_32FC2, flow_pyr);
			BuildPry(sizes, CV_32FC(5), prev_poly_pyr);
			BuildPry(sizes, CV_32FC(5), poly_pyr);

			xyScaleTracks.resize(scale_num);

			frame.copyTo(image);
			cvtColor(image, prev_grey, CV_BGR2GRAY);
			for (int iScale = 0; iScale < scale_num; iScale++)
			{
				if (iScale == 0)
					prev_grey.copyTo(prev_grey_pyr[0]);
				else
					resize(prev_grey_pyr[iScale - 1], prev_grey_pyr[iScale], prev_grey_pyr[iScale].size(), 0, 0, INTER_LINEAR);

				// dense sampling feature points
				std::vector<Point2f> points(0);
				DenseSample(prev_grey_pyr[iScale], points, quality, min_distance);

				// save the feature points
				std::list<Track> &tracks = xyScaleTracks[iScale];
				for (i = 0; i < points.size(); i++)
				{
					tracks.push_back(Track(points[i], trackInfo, hogInfo, hofInfo, mbhInfo));
				}
			}

			// compute polynomial expansion
			my::FarnebackPolyExpPyr(prev_grey, prev_poly_pyr, fscales, 7, 1.5);

			frame_num++;
			continue;
		}

		init_counter++;
		frame.copyTo(image);
		cvtColor(image, grey, CV_BGR2GRAY);

		// compute optical flow for all scales once
		my::FarnebackPolyExpPyr(grey, poly_pyr, fscales, 7, 1.5);
		my::calcOpticalFlowFarneback(prev_poly_pyr, poly_pyr, flow_pyr, 10, 2);

		//get the id for each subject
		x1.clear();
		x2.clear();
		yy1.clear();
		y2.clear();
		
		for (int i = 0; i < new_PAR[frame_num].size(); i++)
		{
			if(new_PAR[frame_num][i]==curSub)
			{
				int x1_ind = (new_PAR[frame_num][i] - 1) * 7 + 1;
				x1.push_back(new_DATA[frame_num][x1_ind]);
				yy1.push_back(new_DATA[frame_num][x1_ind + 1]);
				x2.push_back(new_DATA[frame_num][x1_ind + 2]);
				y2.push_back(new_DATA[frame_num][x1_ind + 3]);
			}
		}
		for (int iScale = 0; iScale < scale_num; iScale++)
		{
			if (iScale == 0)
				grey.copyTo(grey_pyr[0]);
			else
				resize(grey_pyr[iScale - 1], grey_pyr[iScale], grey_pyr[iScale].size(), 0, 0, INTER_LINEAR);

			int width = grey_pyr[iScale].cols;
			int height = grey_pyr[iScale].rows;

			// compute the integral histograms
			DescMat *hogMat = InitDescMat(height + 1, width + 1, hogInfo.nBins);
			HogComp(prev_grey_pyr[iScale], hogMat->desc, hogInfo);

			DescMat *hofMat = InitDescMat(height + 1, width + 1, hofInfo.nBins);
			HofComp(flow_pyr[iScale], hofMat->desc, hofInfo);

			DescMat *mbhMatX = InitDescMat(height + 1, width + 1, mbhInfo.nBins);
			DescMat *mbhMatY = InitDescMat(height + 1, width + 1, mbhInfo.nBins);
			MbhComp(flow_pyr[iScale], mbhMatX->desc, mbhMatY->desc, mbhInfo);

			// track feature points in each scale separately
			std::list<Track> &tracks = xyScaleTracks[iScale];
			for (std::list<Track>::iterator iTrack = tracks.begin(); iTrack != tracks.end();)
			{
				int index = iTrack->index;
				Point2f prev_point = iTrack->point[index];
				int x = std::min<int>(std::max<int>(cvRound(prev_point.x), 0), width - 1);
				int y = std::min<int>(std::max<int>(cvRound(prev_point.y), 0), height - 1);

				Point2f point;
				point.x = prev_point.x + flow_pyr[iScale].ptr<float>(y)[2 * x];
				point.y = prev_point.y + flow_pyr[iScale].ptr<float>(y)[2 * x + 1];

				if (point.x <= 0 || point.x >= width || point.y <= 0 || point.y >= height)
				{
					iTrack = tracks.erase(iTrack);
					continue;
				}

				if (point.x <= 0 || point.x >= width || point.y <= 0 || point.y >= height)
				{
					iTrack = tracks.erase(iTrack);
					continue;
				}
				int con = 0;
				// draw bounding box for all subjects
				for (int i = 0; i < x1.size(); i++)
				{
					if (point.x >= x1[i] && point.x <= x2[i] && point.y >= yy1[i] && point.y <= y2[i])
					{
						con = 1;
					}
				}
				if (con == 0)
				{
					iTrack = tracks.erase(iTrack);
					continue;
				}
				else
				{
					con = 0;
				}

				// get the descriptors for the feature point
				RectInfo rect;
				GetRect(prev_point, rect, width, height, hogInfo);
				GetDesc(hogMat, rect, hogInfo, iTrack->hog, index);
				GetDesc(hofMat, rect, hofInfo, iTrack->hof, index);
				GetDesc(mbhMatX, rect, mbhInfo, iTrack->mbhX, index);
				GetDesc(mbhMatY, rect, mbhInfo, iTrack->mbhY, index);
				iTrack->addPoint(point);

				// draw the trajectories at the first scale
				if (show_track == 1 && iScale == 0)
					DrawTrack(iTrack->point, iTrack->index, fscales[iScale], image);

				// if the trajectory achieves the maximal length
				if (iTrack->index >= trackInfo.length)
				{
					std::vector<Point2f> trajectory(trackInfo.length + 1);
					for (int i = 0; i <= trackInfo.length; ++i)
						trajectory[i] = iTrack->point[i] * fscales[iScale];

					//keep the trajectories positions before normalized
					std::vector<Point2f> trajectory_poisiotns(trackInfo.length + 1);

					for (int i = 0; i <= trackInfo.length; ++i)
						trajectory_poisiotns[i] = iTrack->point[i] * fscales[iScale];

					float mean_x(0), mean_y(0), var_x(0), var_y(0), length(0);
					if (IsValid(trajectory, mean_x, mean_y, var_x, var_y, length))
					{
						//string trajectory_owner;
						//printf("%d\t%f\t%f\t%f\t%f\t%f\t%f\t", frame_num, mean_x, mean_y, var_x, var_y, length, fscales[iScale]);
						//myfile<< frame_num <<"\t"<< mean_x<<"\t"<< mean_y<<"\t"<< var_x<<"\t"<< var_y<<"\t"<< length<<"\t"<< fscales[iScale]<<"\t";
						str.append(tostr(frame_num) + "\t" + tostr(mean_x) + "\t" + tostr(mean_y) + "\t" + tostr(var_x) + "\t" + tostr(var_y) + "\t" + tostr(length) + "\t" + tostr(fscales[iScale]) + "\t");

						// for spatio-temporal pyramid
						//printf("%f\t\n", std::min<float>(std::max<float>(mean_x/float(seqInfo.width), 0), 0.999));
						//printf("%f\t\n", std::min<float>(std::max<float>(mean_y/float(seqInfo.height), 0), 0.999));
						//printf("%f\t\n", std::min<float>(std::max<float>((frame_num - trackInfo.length/2.0 - start_frame)/float(seqInfo.length), 0), 0.999));

						//myfile<<std::min<float>(std::max<float>(mean_x/float(seqInfo.width), 0), 0.999)<<"\t";
						//myfile<<std::min<float>(std::max<float>(mean_y/float(seqInfo.height), 0), 0.999)<<"\t";
						//myfile<<std::min<float>(std::max<float>((frame_num - trackInfo.length/2.0 - start_frame)/float(seqInfo.length), 0), 0.999)<<"\t";

						str.append(tostr(std::min<float>(std::max<float>(mean_x/float(seqInfo.width), 0), 0.999))+"\t");
						str.append(tostr(std::min<float>(std::max<float>(mean_y/float(seqInfo.height), 0), 0.999))+"\t");
						str.append(tostr(std::min<float>(std::max<float>((frame_num - trackInfo.length/2.0 - start_frame)/float(seqInfo.length), 0), 0.999))+"\t");

						//myfile<<iTrack->original_point.x<<"\t"<<iTrack->original_point.y<<"\t";
						str.append(tostr(iTrack->original_point.x) + "\t" + tostr(iTrack->original_point.y) + "\t");

						// output the normalized trajectory (only need to divide by dt to have velocity)
						for (int i = 0; i < trackInfo.length; ++i)
						{
							//printf("%f\t%f\t", trajectory[i].x,trajectory[i].y);
							//myfile<< trajectory[i].x <<"\t"<< trajectory[i].y <<"\t";
							str.append(tostr(trajectory[i].x) + "\t" + tostr(trajectory[i].y) + "\t");
						}

						// output the trajectory positions in x and y
						for (int i = 0; i < trackInfo.length; ++i){
							//printf("%f\t%f\t", trajectory_poisiotns[i].x,trajectory[i].y);
							str.append(tostr(trajectory_poisiotns[i].x) + "\t" + tostr(trajectory_poisiotns[i].y) + "\t");
						}
						
						str.append("\t" + tostr(frame_num*2)+ "\t");
						str.append(PrintDesc(iTrack->hog, hogInfo, trackInfo));
						str.append("\t" + tostr(frame_num*2)+ "\t");
						str.append(PrintDesc(iTrack->hof, hofInfo, trackInfo));
						str.append("\t" + tostr(frame_num*2)+ "\t");
						str.append(PrintDesc(iTrack->mbhX, mbhInfo, trackInfo));
						str.append("\t" + tostr(frame_num*2)+ "\t");
						str.append(PrintDesc(iTrack->mbhY, mbhInfo, trackInfo));
						//printf("\n");
						//myfile<<"\n";

						str.append("\n");
					}

					iTrack = tracks.erase(iTrack);
					continue;
				}
				++iTrack;
			}
			ReleDescMat(hogMat);
			ReleDescMat(hofMat);
			ReleDescMat(mbhMatX);
			ReleDescMat(mbhMatY);

			if (init_counter != trackInfo.gap)
				continue;

			// detect new feature points every initGap frames
			std::vector<Point2f> points(0);
			for (std::list<Track>::iterator iTrack = tracks.begin(); iTrack != tracks.end(); iTrack++)
				points.push_back(iTrack->point[iTrack->index]);

			DenseSample(grey_pyr[iScale], points, quality, min_distance);
			// save the new feature points
			for (i = 0; i < points.size(); i++)
				tracks.push_back(Track(points[i], trackInfo, hogInfo, hofInfo, mbhInfo));
		}

		init_counter = 0;
		grey.copyTo(prev_grey);
		for (i = 0; i < scale_num; i++)
		{
			grey_pyr[i].copyTo(prev_grey_pyr[i]);
			poly_pyr[i].copyTo(prev_poly_pyr[i]);
		}

		//If there are complete trajectories in this frame, store them in a file
		if (!str.empty())
		{
			//printf("Storing file at %d frames... ", frame_num);
			string file_ = string(filename) + "/" + tostr(frame_num) + ".csv";
			//printf (file_,"%d.csv",frame_num);
			//strcat (file_,".csv");
			ofstream myfile;
			ofstream subject;

			myfile.open(file_.c_str());
			myfile << str;
			myfile.close();
			printf("Done!\n");
		}
		if (show_track == 1)
		{
			for (int i = 0; i < new_PAR[frame_num].size(); i++)
			{
				if(new_PAR[frame_num][i] == curSub)
				{
					Point lu = Point(x1[0], yy1[0]);
					Point rt = Point(x2[0], y2[0]);
					// if the current action is annotated as drinking
					char ss[200];
					sprintf(ss, "sub %i", new_PAR[frame_num][i]);
					putText(image, ss, Point(x1[0], yy1[0] + 20), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(255, 0, 0), 2, 1);
					rectangle(image, lu, rt, Scalar(0, 255, 0), 2);
				}
			}
			imshow("DenseTrack", image);
			c = cvWaitKey(3);
			if ((char)c == 27)
				break;
		}
		frame_num++;
	}
	if (show_track == 1)
		destroyWindow("DenseTrack");

	printf("DONE\n");
	//myfile.close();
	return 0;
}

//CSVReader
vector<vector<int> > getData(string fileName)
{
	//Mat new_DATA;
	ifstream inputfile(fileName.c_str());
	string current_line;
	// vector allows you to add data without knowing the exact size beforehand
	vector<vector<int> > all_data;
	// Start reading lines as long as there are lines in the file

	while (getline(inputfile, current_line))
	{
		// Now inside each line we need to seperate the cols
		vector<int> values;
		stringstream temp(current_line);
		string single_value;
		while (getline(temp, single_value, ','))
		{
			// convert the string element to a integer value
			values.push_back(atoi(single_value.c_str()));
		}
		// add the row to the complete data vector
		all_data.push_back(values);
	}
	// Now add all the data into a Mat element (new_DATA 36000*92*7)
	//Mat new_DATA = Mat::zeros(all_data.size(), all_data[0].size(), CV_8UC1);
	// Loop over vectors and add the data

	/*
  for(int rows = 0; rows < all_data.size(); rows++){
          for(int cols = 0; cols<all_data[0].size();cols++){
                  cout<<all_data[rows][cols];
                new_DATA.at<int>(rows,cols) = all_data[rows][cols];
          }
   }
   */
	return all_data;
}
