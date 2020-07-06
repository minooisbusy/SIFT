#include "dataset.h"

namespace DATASET
{
// Functor
bool Dataset::operator()() const
{
    return bValid;
}
// Constructor
Dataset::Dataset(char* _base, char* _scene):base(_base), scene(_scene)
{
    //Validation test
    cout<<"Dataset READ Processing..."<<endl;
    path = base+scene;
    cout<<"input path: "<<path<<endl;
    if(fs::is_directory(path))
    {
        cout<<"Dataset Validation Test Complete"<<endl;
        cout<<"Dataset Path:\n"<<base<<endl;
        cout<<"Scene name:\n"<<scene<<endl;
        bValid = true;

    }
    else
    {
        cout<<"Dataset Validation Fail"<<endl;
        cout<<"Dataset Path:\n"<<base<<endl;
        cout<<"Scene name:\n"<<scene<<endl;
        bValid = false;
        return ;
    }
    iter = fs::directory_iterator(path);
    unsigned int _n_images=0;

    
    count = 0;
    n_images=0;
    {
        fs::directory_iterator end_iter;
        for ( fs::directory_iterator iter(path); iter != end_iter; ++iter)
        {
            if (iter->path().extension() == ".png")
            {
                ++_n_images;
            }
        }
    while (iter != fs::end(iter))
    {
    const fs::directory_entry& entry = *iter;
    filenames.push_back(entry.path());
    ++iter;
    }
    std::sort(filenames.begin(), filenames.end());
   }
    n_images = &_n_images;
    cout<<"Total #image= "<<*n_images<<endl;
    
}

// Methods
std::string Dataset::next()
{
    std::string res = filenames[count];
    count +=1;
    return res;

}
}

