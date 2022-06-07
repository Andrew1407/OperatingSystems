#!/bin/bash

if [[ $1 = 'clear' ]]
then

  rm 6-lab.a gmon.out 6-lab.gprof.txt

else

  c++ 6-lab.cpp -pg -o 6-lab.a -O2 -lc

  ./6-lab.a 1000

  gprof 6-lab.a gmon.out > 6-lab.gprof.txt
  
fi
