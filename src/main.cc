#include"Frame.h"
#include"dataset.h"

int main(int argc, char* argv[])
{
    std::cout<<"--Feature Processing"<<std::endl;
    if(argc<3)
    {
        std::cout<<"usage: ./runfile <filename>"<<std::endl;
        std::cout<<"written parameters:"<<std::endl;
        for(int i=1; i<argc;i++){std::cout<<argv[i]<<std::endl;}
        return 0;
    }
    DATASET::Dataset d(argv[1], argv[2]);
    
    // TODO: Functor error fix
    if(!d())
    {
        cout<<"d()=="<<boolalpha<<d()<<endl;
        std::cout<<"Error exists: directory is not valid"<<std::endl;
        return 0;
    }
    bool verbose = std::strcmp(argv[3], "true")==0?true:false;
    FEATURES::Frame f(d.next());
    
    f.make_candidates(verbose);
    for(int i=0; i<4;i++)
    {
        f.find_ScaleExtrema(i, verbose);
        f.showCands(i);
    }
}
