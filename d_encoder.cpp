// PART D ENCODER
// Description
// This program uses user password seeded random number generator to hide
// consequtive bits of binary message image within randomly chosen bytes of
// noised 3-channel carrier image.

#include <iostream>
#include <algorithm>
#include <vector>
#include <opencv/cv.h>
#include <highgui.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;


unsigned long hash_djb2(const char* str);
void addgaussian(Mat_<Vec3b>& src, Mat_<Vec3b>& dst, double sigma,RNG& rng);

int main(int argc, char* argv[])
{
    if (argc != 4) {  // incorrect number of arguments
        cout << "Error: Lacking arguments" << endl;
        return -1;
    }

    // loading carrier image
    cout << "Loading carrier image (" << argv[1] << ")... ";
    auto carrier = Mat_<Vec3b>{};
    if (!(carrier = imread(argv[1])).data) {
        cout << "Could not open or find " << argv[1] << endl;
        return -1;
    }
    cout << "done" << endl;

    // loading message image
    cout << "Loading message image (" << argv[2] << ")... ";
    auto message = Mat_<uchar>{};
    if (!(message = imread(argv[2], IMREAD_GRAYSCALE)).data) {
        cout << "Could not open or find " << argv[2] << endl;
        return -1;
    }
    cout << "done" << endl;

    // checking dimensions agreement
    if (carrier.size() != message.size()) {
        cout << "Images have different dimensions" << endl;
        return -1;
    }

    // prompting user for a character string password
    cout << "Input password: ";
    string password;
    getline(cin, password);

    // prompting user for sigma value
    cout << "Input sigma: ";
    int sigma;
    cin >> sigma;
    
    // convert into 64 bit integer
    auto seed = hash_djb2(password.c_str());
    RNG rng(seed);
   
    // adding Gaussian noise to the carrier image
    
    cout << "Adding Gaussian noise to the carrier image... ";
    addgaussian(carrier, carrier, sigma, rng);
   
    cout << "done" << endl;

    // bulid a flag matrix to save the status of the pixel
    Mat_<Vec3b> flag(carrier.size(), Vec3b::all(0));

    // distributing message bits over the three colour carrier image chanels
    // iterating through each location in the message image
	cout << "Distributing message bits over carrier image bytes... ";
    auto encoded = carrier.clone();
    // randomly choose a location to hide the message pixel
    for (int j = 0;  j < message.rows; j++)
    {
        for (int i = 0;  i < message.cols;i++)
        {
            //check if the current location has been used or could overflow
            int row, col, channel;
            do {
                row = rng.uniform(0, carrier.rows);
                col = rng.uniform(0, carrier.cols);
                channel = rng.uniform(0, 3);
            } while (flag.at<Vec3b>(row, col)[channel] != 0 || carrier.at<Vec3b>(row, col)[channel] == 255);
        
            encoded.at<Vec3b>(row, col)[channel] += (message.at<uchar>(j,i) ? 0 : 1);
            //set the value of flag 
            flag.at<Vec3b>(row, col)[channel] = 1;
        }
    }
	cout << "done" << endl;

    // saving generated image
    cout << "Saving encoded image (" << argv[3] << ")... ";
	vector<int> compression_params = {CV_IMWRITE_PNG_COMPRESSION, 9};
    imwrite(argv[3], encoded, compression_params);
    cout << "done" << endl;

    // success
    return 0;
}

// hash function
unsigned long hash_djb2(const char* str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}


// in this program, gaussian noise is added to 3 channels
void addgaussian(Mat_<Vec3b>& src, Mat_<Vec3b>& dst, double sigma,RNG& rng)
{
    dst = src.clone();
    for (int j = 0;  j < dst.rows; j++)



    {
        for (int i = 0;  i <dst.cols;i++)
        {
             for (auto n : {0, 1, 2})
             {
                 // avoid overflow
                 if (dst.at<cv::Vec3b>(j,i)[n] != 255)
                 dst.at<cv::Vec3b>(j,i)[n] += rng.gaussian(sigma);
                 if (dst.at<cv::Vec3b>(j, i)[n] < 0) dst.at<cv::Vec3b>(j, i)[n] = 0;
             }
           
        }
    }
}