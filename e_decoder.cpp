// Decoded information

// Description
// This program uses same password to decode information
// It can only decoded the bit&byte message, but can not recover it to the original format

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <memory>
#include <string>

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>


using namespace cv;

using namespace std;

unsigned long hash_djb2(const char* str);
inline char changebit(string data);

void addgaussian(Mat_<Vec3b>& src, Mat_<Vec3b>& dst, double sigma, RNG& rng);

int main(int argc, char* argv[])
{
    if (argc != 4) {  // incorrect number of arguments
        cout << "Please provide correct nmuber of arguments" << endl;
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

    // loading encoded image
    cout << "Loading encoded image (" << argv[2] << ")... ";
    auto encoded = Mat_<Vec3b>{};
    if (!(encoded = imread(argv[2])).data) {
        cout << "Could not open or find " << argv[2] << endl;
        return -1;
    }
    cout << "done" << endl;


    // prompting user for a character string password
    cout << "Input password: ";
    string password;
    getline(cin, password);

    // transforming password string to a 64-bit integer seed (with hash
    // function)
    auto seed = hash_djb2(password.c_str());
    RNG rng(seed);

    // adding Gaussian noise to the carrier image
    // in this case, we use a fixed sigma value

    cout << "Adding Gaussian noise to the carrier image... ";
    double sigma = 10;
    addgaussian(carrier, carrier, sigma, rng);

    cout << "done" << endl;

    // counting number of slots in noised carrier image
    cout << "Counting number of free slots in noised carrier image... ";

    Mat_<Vec3b> flag(carrier.size(), Vec3b::all(0));

    int size = carrier.rows * carrier.cols;
    // creat a index vector for the shuffle program
    vector<int> buf_index(size);
    int index_ini = 0;
    for (auto& index : buf_index) index = index_ini++;

    vector<unsigned char> decoded(size);

    // random shuffling vector of slots in carrier image
    cout << "Shuffling a vector of free slots... ";
    random_shuffle(buf_index.begin(), buf_index.end(), rng);
    cout << "done" << endl;


    // distributing message bits over the three colour carrier image chanels
   // iterating through each location in the message image
    cout << "Distributing message bits over carrier image bytes... ";
    auto encoded = carrier.clone();

    string bit[8];

    // decode message information
    for (int j = 0; j < size; j++)
    {
        int row, col, channel;
        for (int i = 0; i < 8; i++)
        {
            //check if the current location has been used or could overflow

            do {
                row = rng.uniform(0, carrier.rows);
                col = rng.uniform(0, carrier.cols);
                channel = rng.uniform(0, 3);
            } while (flag.at<Vec3b>(row, col)[channel] != 0 || carrier.at<Vec3b>(row, col)[channel] == 255);

            bit[i] = encoded.at<Vec3b>(row, col)[channel] - carrier.at<Vec3b>(row, col)[channel];
            //set the value of flag 
            flag.at<Vec3b>(row, col)[channel] = 1;
        }
        decoded[j] = changebit(bit);
        bit->clear();

    }

    cout << "done" << endl;



    // saving decoded message
    cout << "Saving decoded message (" << argv[3] << ")... ";
    auto file = ofstream(argv[3], ios::binary | ios::trunc);
    if (!file.is_open()) {
        cout << "Could not open or find " << argv[3] << endl;
        return -1;
    }
    
    ostream_iterator<char> output_iterator(file, "\n");
    copy(decoded.begin(), decoded.end(), output_iterator);
    cout << "done" << endl;

    
    // success
    return 0;
}


unsigned long hash_djb2(const char* str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}


//reverse bit to byte
inline char changebit(string* data)
{
        char result;
        char c = '\0';
        for (int i = 0; i < 8; i++)
        {
            if (data[i] == '1') c = (c << 1) | 1;
            else c = c << 1;
        }
     
        result += (unsigned char)c;
        return  result;
    }
 


// in this program, gaussian noise is added to 3 channels
void addgaussian(Mat_<Vec3b>& src, Mat_<Vec3b>& dst, double sigma, RNG& rng)
{
    dst = src.clone();
    for (int j = 0; j < dst.rows; j++)
    {
        for (int i = 0; i < dst.cols; i++)
        {
            for (auto n : { 0, 1, 2 })
            {
                // avoid overflow
                if (dst.at<cv::Vec3b>(j, i)[n] != 255)
                    dst.at<cv::Vec3b>(j, i)[n] += rng.gaussian(sigma);
                if (dst.at<cv::Vec3b>(j, i)[n] < 0) dst.at<cv::Vec3b>(j, i)[n] = 0;
            }

        }
    }
}