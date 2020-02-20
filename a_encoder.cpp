// PART A ENCODER
// Program adds a binary image to a secondary carrier image in order to conceal
// the bit-mapped text message contained in the binary image.
#include <iostream>
#include <algorithm>
#include <vector>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

Mat encoding( Mat message, Mat carrier);

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

    // loading message image
    cout << "Loading message image (" << argv[2] << ")... "; //print name of the message image
    Mat message = Mat(imread(argv[2], cv::IMREAD_GRAYSCALE));
    if (!message.data) {
        cout << "Could not open or find " << argv[2] << endl;
        return -1;
    }
    cout << "Completed" << endl;

    // Check if dimensions of two image agree
    cout << "Checking dimension agreement... ";
    if (carrier.size == message.size)
        cout << "Agree" << endl;
    else{
        cout << "Disagree" << endl;
        cout << "ERROR: Images have different dimension" << endl;
        return -1;
    }

    // generating encoded image
    cout << "Generating encoded image... ";
    // convert grayscale to binary  *notice that 0 -> black; 1 -> white
    threshold(message, message, 0, 1, THRESH_BINARY);
    // revert 0 and 1 *now 0 -> white; 1 -> black
    message = Mat_<uchar>::ones(message.size()) - message;

    Mat enc = encoding(carrier, message);


   
    cout << "Completed" << endl;

    // saving generated image
    cout << "Saving encoded image (" << argv[3] << ")... ";


    vector<int> compression_params = {cv::IMWRITE_PNG_COMPRESSION, 9};
    imwrite(argv[3], enc, compression_params);
    cout << "Completed" << endl;

    // success
    return 0;
}

Mat encoding( Mat message, Mat carrier)
{
    //Create a same but empty Mat to store encoded image
    Mat enc;
    enc.create(message.size(),message.type()); //
    for (int j = 0; j < message.cols; j++)
    {
        for (int i = 0; i < message.rows; i++)
        {
            if (carrier.at<uchar>(i, j) == 255)
                enc.at<uchar>(i, j) = carrier.at<uchar>(i, j) - message.at<uchar>(i, j);
            else
                enc.at<uchar>(i,j) = carrier.at<uchar>(i, j) + message.at<uchar>(i, j);
        }

    }
    return enc;
}