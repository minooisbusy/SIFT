#ifndef _FRMAE_H_
#define _FRAME_H_
#define uint32 unsigned int
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

using namespace cv;
namespace FEATURES
{
template<typename ... Args>
std::string string_format(const std::string& format, Args ... args);
class Frame 
{
private:
    //Variables
    Mat src;
    Mat gray;
    Mat padded;
    Mat **scales;
    Mat **dog;
    uint32 n_scales;
    uint32 n_octaves;
    cv::Size sz_kernel;
    double sigma;
    //Methods
    
public:
    Frame();
    Frame(std::string filename, int n_octave=4, int n_scale=5);
    void const getImage();
    bool readImage(std::string filename);
    void showImage(std::string winname);
    void make_candidates(bool verbose = false);
    void find_ScaleExtrema(int octave);
    ~Frame();
    
};
}

#endif