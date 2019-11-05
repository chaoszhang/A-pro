#!/bin/bash

if [ ! -f GenetreeAnnotator ]; then
    g++ -std=c++11 GenetreeAnnotator.cpp -o GenetreeAnnotator || icc -std=c++11 GenetreeAnnotator.cpp -o GenetreeAnnotator
fi

python preprocessing.py $1 $3 > genetrees.txt
./GenetreeAnnotator
java -D"java.library.path=ASTRAL-MP/lib" -jar ASTRAL-MP/astral.5.14.2.jar -C -i breaked_trees.tre -f cluster_trees.tre -o $2