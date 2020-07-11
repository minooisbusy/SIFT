cd build && make
cd ../bin
if [ $# -ne 1 ]
then echo "plz check argiments again"
else
    echo "script is running"
    ./sift ../data/kitti/ stuttgart_00 $1
fi
