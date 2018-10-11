#include <iostream> // for standard I/O
#include <string>   // for strings

#include <opencv2/core/core.hpp>        // Basic OpenCV structures (cv::Mat)
#include <opencv2/highgui/highgui.hpp>  // Video write

#include <fstream>
#include <sys/stat.h>

using namespace std;
using namespace cv;

static void help()
{
    cout
            << "------------------------------------------------------------------------------" << endl
            << "This program shows how to write video files."                                   << endl
            << "You can extract a portition of the input video."              << endl
            << "Usage:"                                                                         << endl
            << "./video-write inputvideoName starting_frame_no endframe_no"                              << endl
            << "------------------------------------------------------------------------------" << endl
            << endl;
}

vector<vector<int> > getData(string filename);

string DRINK_DAY1_CAM12_path = "/Users/Andyteng/Dropbox/Graduation_Project/MatchMakersStudentEdition/annotations/partitions/drinking/drinking_DAY3_CAM12.csv";
vector<vector<int> > new_DRINK_DAY1 = getData(DRINK_DAY1_CAM12_path);

int main(int argc, char *argv[])
{
    help();

    //if (argc != 4)
    //{
     //   cout << "Not enough parameters" << endl;
      //  return -1;
    //}

    const string source = argv[1];           // the source file name

    int start_frame_no = 0; // convert our input string to number - C++ style
    //stringstream s;
    //s << argv[2];
    //s >> start_frame_no;


    int end_frame_no = 0;
    //stringstream s2;
    //s2 << argv[3];
    //s2 >> end_frame_no;

    VideoCapture inputVideo(source);              // Open input
    if (!inputVideo.isOpened())
    {
        cout  << "Could not open the input video: " << source << endl;
        return -1;
    }
    string NAME = "DAY1_CAM12_drinking.mp4";
    //string::size_type pAt = source.find_last_of('.');                  // Find extension point
    //const string NAME = source.substr(0, pAt) + argv[2] + "-" +argv[3]+".mp4";   // Form the new name
    int ex = static_cast<int>(inputVideo.get(CV_CAP_PROP_FOURCC));     // Get Codec Type- Int form

    // Transform from int to char via Bitwise operators
    char EXT[] = {(char)(ex & 0XFF) , (char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24), 0};

    Size S = Size((int) inputVideo.get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
                  (int) inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT));

    VideoWriter outputVideo;                                        // Open the output

        outputVideo.open(NAME, ex, inputVideo.get(CV_CAP_PROP_FPS), S, true);

    if (!outputVideo.isOpened())
    {
        cout  << "Could not open the output video for write: " << source << endl;
        return -1;
    }

    cout << "Input frame resolution: Width=" << S.width << "  Height=" << S.height
         << " of nr#: " << inputVideo.get(CV_CAP_PROP_FRAME_COUNT) << endl;
    cout << "Input codec type: " << EXT << endl;


    Mat src;


    inputVideo.set(CV_CAP_PROP_POS_FRAMES,start_frame_no);

    for(;;) //Show the image captured in the window and repeat
    {

        inputVideo >> src;              // read

        if(src.empty() || start_frame_no> 36000) break;         // check if at end
        //outputVideo.write(src); //save or
        if(!new_DRINK_DAY1[start_frame_no].empty())
        {
            outputVideo << src;
        }
        start_frame_no++;
    }

    cout << "Finished writing" << endl;
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
