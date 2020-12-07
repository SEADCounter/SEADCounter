#/bin/sh

g++ -std=c++11 statistic.cpp

for n in  06  12  18  24
do
	echo "processing 0${n}.dat"
	./a.out 0${n}.dat 0${n}.stat
done

