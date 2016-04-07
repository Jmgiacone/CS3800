#!/bin/bash

for i in 1 2 4 8 16
do
  for j in "lru" "fifo" "clock"
  do
    for k in "d" "p"
    do
      echo ./driver programlist programtrace $i $j $k
      ./driver programlist programtrace $i $j $k
    done
  done
done
