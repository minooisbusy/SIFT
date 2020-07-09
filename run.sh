cd build && make
cd ../bin
if [ $# -ne 1 ]
then echo "plz check argiments again"
else
    echo "script is running"
    ./sift ../data/kitti/ image_2 $1
fi
