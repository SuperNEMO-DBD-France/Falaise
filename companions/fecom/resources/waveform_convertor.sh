#!/bin/bash

i=0
for value in $(<raw_waveform3_from_run40.txt)
do
    echo "$i $value"
    ((i+=1))
done
