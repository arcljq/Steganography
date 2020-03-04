// General Information Hiding - encoder
// Usage: program_name carrier message encoded

// Description
// This program uses user password seeded random number generator to hide
// consequtive bits of user selected file within randomly chosen bytes of
// noised 3-channel carrier image.

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <memory>

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;

using namespace std;

unsigned long hash_djb2(const char* str);
template <typename T>
inline bool get_bit(T& var, unsigned n);

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

    // loading message file
    // read file as binary, locate the point of the reader to the end of file
    cout << "Loading message file (" << argv[2] << ")... ";
    auto file = ifstream(argv[2], ios::binary | ios::ate);
    if (!file.is_open()) {
        cout << "Could not open or find " << argv[2] << endl;
        return -1;
    }

    // create a vector to store the bits read from the file
    vector<unsigned char> buffer;
    long size = file.tellg();

    // relocate the point to the begin of the file
    file.seekg(0, ios::beg);

    buffer.resize(size);
    // read byts into file
    file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
    cout << "done (" << size * 8 << " bits)" << endl;

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

    // creat a index vector for the shuffle program
    vector<int> buf_index(size);
    int index_ini = 0;
    for (auto& index : buf_index) index = index_ini++;
    
  
    // random shuffling vector of slots in carrier image
    cout << "Shuffling a vector of free slots... ";
    random_shuffle(buf_index.begin(), buf_index.end(), rng);
    cout << "done" << endl;

  
    // distributing message bits over the three colour carrier image chanels
   // iterating through each location in the message image
    cout << "Distributing message bits over carrier image bytes... ";
    auto encoded = carrier.clone();
    
    // hide message information
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

            encoded.at<Vec3b>(row, col)[channel] += get_bit(buffer[buf_index[j]], i);
            //set the value of flag 
            flag.at<Vec3b>(row, col)[channel] = 1;
        }
        
    }
  

    cout << "done" << endl;

   

    // saving generated image
    cout << "Saving generated image (" << argv[3] << ")... ";
    vector<int> compression_params = { CV_IMWRITE_PNG_COMPRESSION, 9 };
    imwrite(argv[3], encoded, compression_params);
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

template <typename T>
inline bool get_bit(T& var, unsigned n)
{
    
    unsigned char b;
    b = 1 & (val >> n);
    return b;
    
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