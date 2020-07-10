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
    sigma = 1.6f;
    Extrema = new std::vector<cv::Point2f>[n_octaves];

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
    if(Extrema !=nullptr)
        delete [] Extrema;
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

    sz_kernel = Size(0,0);//Arbitrary size by sigma magnitude.
    const double k_divide = 2.0;
    double legacy_sigma=0;
    double sigma_scale = this->sigma;
    // First scale in first octave is original
    //scales[0][0] = gray.clone();
    cv::GaussianBlur(gray,
                    scales[0][0],
                    sz_kernel,
                    sigma_scale,
                    sigma_scale);
    
    double k = sqrt(2);//pow(2.0f,1.0/(n_scales-1));
    for(uint32 octave=0; octave<n_octaves; octave++)
    {
            if(verbose)
            {
            std::cout<<"\t\t scale:"<<0<<", total sigma="<<legacy_sigma<<", cur sigma="<<sigma_scale<<std::endl;
            }
        sigma_scale = this->sigma;
        for(uint32 scale=0; scale<n_scales-1; scale++)
        {
                cv::GaussianBlur(scales[octave][scale],
                                 scales[octave][scale+1],
                                 sz_kernel,
                                 sigma_scale, //TODO
                                 sigma_scale);
            sigma_scale =pow(k,scale)*sigma_scale;
            if(verbose)
            {
            legacy_sigma = sigma_compute(legacy_sigma, sigma_scale);
            std::cout<<"\t\t scale:"<<scale+1<<", total sigma="<<legacy_sigma<<", cur sigma="<<sigma_scale<<std::endl;
            }
        }

        //Next octave before last octave
        if( octave < n_octaves-1)
        {
            if(verbose)
                std::cout<<"\t\t next"<<std::endl;
            cv::resize(scales[octave][n_scales-3],
                       scales[octave+1][0],
                       Size(int(scales[octave][n_scales-1].cols/k_divide+1), 
                            int(scales[octave][n_scales-1].rows/k_divide+1)));
        }
    }
    for(uint32 octave=0; octave<n_octaves; octave++)
    {
        for(uint32 scale=0; scale<n_scales-1; scale++)
        {
            dog[octave][scale] = scales[octave][scale+1] - scales[octave][scale];
            //std::cout<<string_format("Size of dog[%d][%d], height = %d, width= %d",octave,scale, dog[octave][scale].rows, dog[octave][scale].cols)<<std::endl;
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
                double min, max;
                cv::minMaxLoc(dog[octave][scale],&min,&max);
                Mat normalized = ((dog[octave][scale]-min)/(max-min));
                normalized.convertTo(normalized,CV_8U, 255,0);
                String name = string_format("octave:%d, scale:%d-%d", octave, scale+1, scale); 
                //imshow(name, dog[octave][scale]);
                imshow(name, normalized);
            }
                waitKey(0);
                cv::destroyAllWindows();
        }
    }

    // 4. Accurate Keypoint localization
}
void Frame::find_ScaleExtrema(int octave, bool verbose)
{
    std::cout<<"Start Local Extrema Detection"<<std::endl;
    //const Mat &src = dd
    // 9+9+8=26, 1
    Size sz = dog[octave][0].size();
    std::cout<<sz<<std::endl;
    Point location;
    double neighbor[27];
   // std::vector<cv::Point2f> Extrema;
    
    for(unsigned int x=1; x<sz.height-1; x++)
    {
        //top
        for(uint32 y=1; y<sz.width-1; y++)
        {
            if(verbose)
            std::cout<<"center = ("<<x<<", "<<y<<")"<<std::endl;
            for(uint32 idx_dog=0; idx_dog<n_scales-3; idx_dog++) // select base ...?
            {
                for(unsigned int scale=idx_dog; scale<idx_dog+3;scale++)// Selected relative 3 scales
                {
                    for( int dx=-1; dx<2; dx++)
                    {
                        for(int dy=-1; dy<2; dy++)
                        {
                            int idx = (1+dx)*3+(1+dy)+9*(scale-idx_dog);
                            neighbor[idx] = abs(dog[octave][scale].at<float>(x+dx,y+dy));
                            if(verbose)
                            std::cout<<string_format("octave = %d, scale = %d, layer = %d, idx = %d, (%d,%d), value = %lf",octave, scale,idx_dog, idx,dx,dy,neighbor[idx])<<std::endl;
                        }
                    }
                }
                bool bExtrema = false;
                for(int i=0;i<27;i++)
                {
                    if(neighbor[13]>0)
                    {
                        if(neighbor[13]>=neighbor[i])
                        {
                            bExtrema=true;
                        }
                        else
                        {
                            bExtrema=false;
                            break;
                        }
                    }
                    else if(neighbor[13]<0)
                    {
                        if(neighbor[13]<=neighbor[i])
                        {
                            bExtrema=true;
                        }
                        else
                        {
                            bExtrema=false;
                            break;
                        }
                    }
                }
                if(bExtrema)
                {
                    Extrema[octave].push_back(Point(y,x));
                    //std::cout<<string_format("Point(%d,%d) is extrema",x,y)<<std::endl;
                }
                else
                {
                    if(verbose)
                    std::cout<<string_format("Point(%d,%d) is NOT extrema",x,y)<<std::endl;
                }
            }
        }
    }
    std::cout<<"octave:"<<octave<<", #candiates="<<Extrema[octave].size()<<std::endl;
}
void Frame::showCands(int octave)
{
    Mat shows =scales[octave][0].clone();
    cv::cvtColor(shows,shows,COLOR_GRAY2RGB);
    for(int i=0; i<Extrema[octave].size(); i++)
        cv::circle(shows, Extrema[octave][i], 3, Scalar(0,0,255), 1);
    imshow("cands", shows);
    waitKey(0);
    destroyAllWindows();
}
void Frame::process(bool verbose)
{
    make_candidates(verbose);
    for(int i=0;i<n_octaves; i++)
    {
        find_ScaleExtrema(i, verbose);
    }
}
// getValue
float Frame::getValue(const Mat& src, const int x, const int y)
{
    return src.at<float>(x, y);
}


//Derivatives
Point3f Frame::foDerivative(int octave, int scale, Point2f p)
{
    assert(scale>1);
    assert(p.x>=1||p.y>=1);
    const float dx = getValue(dog[octave][scale],p.x-1, p.y)
                    -getValue(dog[octave][scale],p.x+1, p.y);

    const float dy = getValue(dog[octave][scale],p.x, p.y-1)
                    -getValue(dog[octave][scale],p.x, p.y+1);

    const float ds = getValue(dog[octave][scale-1],p.x, p.y)
                    -getValue(dog[octave][scale+1],p.x, p.y);
    return Point3f(dx,dy,ds);
}
//Reference site: https://github.com/snowiow/SIFT
Mat Frame::soDerivative(int octave, int scale, Point2f p)
{
    assert(scale>1);
    assert(p.x>=1||p.y>=1);

    const float dxx = getValue(dog[octave][scale],p.x-1, p.y)
                     +getValue(dog[octave][scale],p.x+1, p.y)
                     -2*getValue(dog[octave][scale],p.x, p.y);

    const float dyy = getValue(dog[octave][scale],p.x, p.y-1)
                     +getValue(dog[octave][scale],p.x, p.y+1)
                     -2*getValue(dog[octave][scale],p.x, p.y);

    const float dss = getValue(dog[octave][scale-1],p.x, p.y)
                     +getValue(dog[octave][scale+1],p.x, p.y)
                     -2*getValue(dog[octave][scale],p.x, p.y);

    const float dxy = (getValue(dog[octave][scale],p.x+1, p.y+1)
                      +getValue(dog[octave][scale],p.x-1, p.y+1)
                      +getValue(dog[octave][scale],p.x+1, p.y-1)
                      -getValue(dog[octave][scale],p.x-1, p.y-1))/2.0;

    const float dxs =(getValue(dog[octave][scale+1],p.x+1, p.y)
                     +getValue(dog[octave][scale+1],p.x-1, p.y)
                     +getValue(dog[octave][scale-1],p.x+1, p.y)
                     -getValue(dog[octave][scale-1],p.x-1, p.y))/2.0;

    const float dys =(getValue(dog[octave][scale+1],p.x, p.y+1)
                     +getValue(dog[octave][scale+1],p.x, p.y-1)
                     +getValue(dog[octave][scale-1],p.x, p.y+1)
                     -getValue(dog[octave][scale-1],p.x, p.y-1))/2.0;
    Mat result = Mat::zeros(3,3,CV_32F);
    result.at<float>(0,0) = dxx;
    result.at<float>(0,1) = dxy;
    result.at<float>(0,2) = dxs;
    result.at<float>(1,0) = dxy;
    result.at<float>(1,1) = dyy;
    result.at<float>(1,2) = dys;
    result.at<float>(2,0) = dxs;
    result.at<float>(2,1) = dys;
    result.at<float>(2,2) = dss;
    return result;
}

}
