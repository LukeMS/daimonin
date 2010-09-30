#!/bin/bash
echo
echo Choose the build type [D]ebug or [R]elease?
echo "(If you just want to play the game choose Release)"
read -n1 taste
echo $taste
if [ "$taste" = "r" -o "$taste" = "R" ]; then
cmake -G"CodeBlocks - Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ../../../
else
cmake -G"CodeBlocks - Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug ../../../
fi
