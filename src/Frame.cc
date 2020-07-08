#include"Frame.h"

namespace FEATURES
{
template<typename ... Args> 
std::string string_format(const std::string& format, Args ... args) 
{ 
    size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1; 
    if (size <= 0)
    { 
        throw std::runtime_error("Error during formatting."); 
    }
    std::unique_ptr<char[]> buf(new char[size]); 
    snprintf(buf.get(), size, format.c_str(), args ...); 
    return std::string(buf.get(), buf.get() + size - 1); 
}

double sigma_compute(double s1, double s2)
{
    return cv::sqrt(cv::pow(s1,2)+cv::pow(s2,2));
}

bool Frame::readImage(std::string filename)
{
    std::cout<<"Read image: "<<filename<<std::endl;
    this->src = imread(filename, IMREAD_COLOR);
    cvtColor(src,gray,COLOR_BGR2GRAY);
    gray.convertTo(gray, CV_32F, 1.f/255);
    if(src.data == NULL)
    {
        std::cout<<"Error:readImage \nFilename:"<<filename<<std::endl;
        return -1;
    }
    return 0;
};
Frame::Frame(){};
Frame::Frame(std::string filename, int n_octave, int n_scale):scales(nullptr),n_scales(n_scale), n_octaves(n_octave) 
{ 
    sigma = 1.f;

    //Parallel cases
    /*
    arr_sigma = new double[n_scales];
    arr_sigma[0] = 1.0;
    for(uint32 i=1; i<n_scales; i++)
    {
        arr_sigma[i] = arr_sigma[i-1]*sigma; // sigma \ast \sigma
    }
    */
    /*
    for(uint32 i=0; i<n_scales; i++)
        std::cout<<"sigma["<<i<<"] = "<<arr_sigma[i]<<std::endl;
    */


    readImage(filename);
}
Frame::~Frame()
{
    /*
    if(arr_sigma !=nullptr)
        delete [] arr_sigma;
    */
    if(scales != nullptr)
    {
        std::cout<<"Dynamic array dealloc:scale"<<std::endl;
        for(uint32 i=0; i<n_octaves; i++)
        {
            delete [] scales[i];
        }
        delete[] scales;
    }
    if(dog != nullptr)
    {
        std::cout<<"Dynamic array dealloc:DoG"<<std::endl;
        for(uint32 i=0; i<n_octaves; i++)
        {
            delete [] dog[i];
        }
        delete[] dog;
    }
}
void Frame::showImage(std::string winname)
{
    imshow(winname, src);
    waitKey(0);
}
void Frame::make_candidates(bool verbose)
{
    // Dynamically allocation: octave-scale
    scales = new Mat*[n_octaves];
    for(uint32 i=0; i<n_octaves; i++)
    {
        scales[i] = new Mat[n_scales];
    }
    // Dynamically allocation: octave-dog
    dog = new Mat*[n_octaves];
    for(uint32 i=0; i<n_octaves; i++)
    {
        dog[i] = new Mat[n_scales-1];
    }

    sz_kernel = Size(5,5);
    const double k_divide = 2.0;
    double legacy_sigma=0;


    // First scale in first octave is original
    scales[0][0] = gray.clone();
    //cv::GaussianBlur(gray,
    //                scales[0][0],
    //                sz_kernel,
    //                sigma_scale,
    //                sigma_scale);
    
    double k = pow(2.0f,1.0/(n_scales-1));
    double sigma_scale = this->sigma;
    for(uint32 octave=0; octave<n_octaves; octave++)
    {
        for(uint32 scale=0; scale<n_scales-1; scale++)
        {
            std::cout<<"\t\t scale:"<<scale<<", total sigma="<<legacy_sigma<<", cur sigma="<<sigma_scale<<std::endl;
                cv::GaussianBlur(scales[octave][0],
                                 scales[octave][scale+1],
                                 sz_kernel,
                                 sigma_scale, //TODO
                                 sigma_scale);
            sigma_scale *=k;
            legacy_sigma = sigma_compute(legacy_sigma, sigma_scale);
        }
            std::cout<<"\t\t scale:"<<4<<", total sigma="<<legacy_sigma<<", cur sigma="<<sigma_scale<<std::endl;

        //Next octave before last octave
        if( octave < n_octaves-1)
        {
            std::cout<<"\t\t next"<<std::endl;
                cv::resize(scales[octave][n_scales-1],
                           scales[octave+1][0],
                           Size(int(scales[octave][n_scales-1].cols/k_divide), 
                                int(scales[octave][n_scales-1].rows/k_divide)));
        }
    }
    for(uint32 octave=0; octave<n_octaves; octave++)
    {
        for(uint32 scale=0; scale<n_scales-1; scale++)
        {
            dog[octave][scale] = scales[octave][scale+1] - scales[octave][scale];
        }
    }
    
    if(verbose)
    {
        for(uint32 octave=0; octave<n_octaves; octave++)
        {
            for(uint32 scale=0; scale<n_scales; scale++)
            {
                String name = string_format("octave:%d, scale:%d", octave, scale); 
                imshow(name,scales[octave][scale]);
            }
            waitKey(0);
            cv::destroyAllWindows();
        }
        for(uint32 octave=0; octave<n_octaves; octave++)
        {
            for(uint32 scale=0; scale<n_scales-1; scale++)
            {
                String name = string_format("octave:%d, scale:%d-%d", octave, scale+1, scale); 
                imshow(name, dog[octave][scale]);
            }
                waitKey(0);
                cv::destroyAllWindows();
        }
    }

    // 4. Accurate Keypoint localization
    


}
void Frame::find_ScaleExtrema(int octave)
{
    std::cout<<"Start Local Extrema Detection"<<std::endl;
    //const Mat &src = dd
    // 9+9+8=26, 1
    Size sz = dog[octave][0].size();
    std::cout<<sz<<std::endl;
    Point location;
    double neighbor[26];
    
    for(unsigned int x=1; x<sz.height-1; x++)
    {
        //top
        for(uint32 y=1; y<sz.width-1; y++)
        {
            std::cout<<"center = ("<<x<<", "<<y<<")"<<std::endl;
            std::cout<<"n_scales="<<n_scales<<std::endl;
            for(uint32 idx_dog=0; idx_dog<n_scales-2; idx_dog++) // select base ...?
            {
                for(unsigned int scale=idx_dog; scale<idx_dog+3;scale++)// Selected relative 3 scales
                {
                    for( int dx=-1; dx<2; dx++)
                    {
                        for(int dy=-1; dy<2; dy++)
                        {
                            int idx = 2+x+y+9*(scale-idx_dog);
                            neighbor[idx] = dog[octave][scale].at<float>(y,x);
                            string_format("octave = %d, scale = %d, idx = %d \n",octave, scale, idx);
                        }
                    }
                    return;
                }
            }
        }
    }
        
}

}
