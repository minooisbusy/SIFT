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

bool Frame::readImage(std::string filename)
{
    std::cout<<"Read image: "<<filename<<std::endl;
    this->src = imread(filename, IMREAD_COLOR);
    cvtColor(src,gray,COLOR_BGR2GRAY);
    gray.convertTo(gray, CV_32F, 1.f/255);
    double min, max;
    cv::minMaxLoc(gray, &min, &max);
    std::cout<<"gray min, max = "<<min<<", "<<max<<std::endl;
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
    sigma = cv::pow(2, 1.0/(n_scale-1));
    readImage(filename);
}
Frame::~Frame()
{
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
    double k=1.0;
    double k_divide = 2;

    // First scale in first octave is original
    //scales[0][0] = gray.clone();
    cv::GaussianBlur(gray,
                     scales[0][0],
                     sz_kernel,
                     k,
                     k);
    
    for(uint32 octave=0; octave<n_octaves; octave++)
    {
        k=1.0f;
        std::cout<<"process> octave:"<<octave<<", k="<<k<<std::endl;
        //each scale
        for(uint32 scale=0; scale<n_scales-1; scale++)
        {
            k *= sigma;
            std::cout<<"\t\t scale:"<<scale+1<<", k="<<k<<std::endl;
                cv::GaussianBlur(scales[octave][scale],
                                 scales[octave][scale+1],
                                 sz_kernel,
                                 k,
                                 k);
        }
        //Next octave
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

    // 4. Accurate Keypoint localization
    


}
void Frame::find_ScaleExtrema(int octave)
{
    //const Mat &src = dd
    // 9+9+8=26, 1
    Size sz = dog[octave][0].size();
    Point location;
    double val_loc;
    double neighbor[26];
    
    for(uint32 idx_dog=0; idx_dog<n_scales-2; idx_dog++)
    {
        //top
        for(uint32 scale=idx_dog; scale<idx_dog+3;scale++)
        {
        for(uint32 x=1; x<sz.height-1; x++)
            for(uint32 y=1; y<sz.width-1; y++)
            {
                
                location = Point(x,y);
                val_loc = scales[octave][idx_dog+1].at<float>(y,x);
                for(int i=0; i<27;i++)
                    // int(data_idx/width) == y
                    // data_idx%width = x
                    neighbor[0] = scales[octave][idx_dog+1].at<float>(y-1,x-1);
                    neighbor[1] = scales[octave][idx_dog+1].at<float>(y-1,x-1);

                
            }
        }
    }
        
    }
}

}
