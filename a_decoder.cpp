// PART A DECODER
// Program recovers the original text from an encoded image produced with
// encoder.



#include <iostream>
#include <algorithm>
#include <vector>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

Mat decoding( Mat encoded, Mat carrier);

int main(int argc, char* argv[])
{
    if (argc != 4) {  // incorrect number of arguments
        cout << "Error: incorrect number of arguments" << endl;
        return -1;
    }

    // loading carrier image
    cout << "Loading carrier image (" << argv[1] << ")... ";  //print name of the carrier image

    Mat carrier = Mat(imread(argv[1], cv::IMREAD_GRAYSCALE)); //open the image as grayscale


    if (!carrier.data) {
        cout << "Fail to open the file " << argv[1] << endl;
        return -1;
    }
    cout << "Completed" << endl;

    // loading encoded image
    cout << "Loading encoded image (" << argv[2] << ")... "; //print name of the decoded image
    Mat encoded = Mat(imread(argv[2], cv::IMREAD_GRAYSCALE));
    if (!encoded.data) {
        cout << "Could not open or find " << argv[2] << endl;
        return -1;
    }
    cout << "Completed" << endl;

    // Check if dimensions of two image agree
    cout << "Checking dimension agreement... ";
    if (carrier.size == encoded.size)
        cout << "Agree" << endl;
    else{
        cout << "Disagree" << endl;
        cout << "ERROR: Images have different dimension" << endl;
        return -1;
    }

    // generating encoded image
    cout << "Generating encoded image... ";


    Mat dec = decoding(carrier, encoded);



    cout << "Completed" << endl;

    // saving generated image
    cout << "Saving encoded image (" << argv[3] << ")... ";


    vector<int> compression_params = {cv::IMWRITE_PNG_COMPRESSION, 9};
    imwrite(argv[3], dec, compression_params);
    cout << "Completed" << endl;

    // success
    return 0;
}

Mat decoding( Mat encoded, Mat carrier)
{
    //Create a same but empty Mat to store encoded image
    Mat dec = Mat_<uchar>::ones(encoded.size()) ;

    for (int j = 0; j < encoded.cols; j++)
    {
        for (int i = 0; i < encoded.rows; i++)
        {
            if ((encoded.at<uchar>(i, j) - carrier.at<uchar>(i, j)) < 0)
                dec.at<uchar>(i, j) -= carrier.at<uchar>(i, j) -encoded.at<uchar>(i, j);
            else
                dec.at<uchar>(i, j) -= encoded.at<uchar>(i, j) - carrier.at<uchar>(i, j);
        }

    }

    dec *= 255; //
    return dec;
}


