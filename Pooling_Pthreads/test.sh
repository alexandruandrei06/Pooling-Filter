#!/bin/bash

echo '-----------------Serial-------------------' > results.txt
echo "" >> results.txt
srun -p hpsl -c 1 ../Tema_APP_Serial/Pooling ../input/test10.bmp result.bmp >> results.txt

echo "" >> results.txt
echo '-----------------Paralel------------------' >> results.txt
echo "" >> results.txt

for i in 2 4 8 16 20 28 32 36 40
do
    echo "$i threads: " >> results.txt
    srun -p hpsl -c $i ./Pooling $i ../input/test10.bmp result.bmp >> results.txt
    echo "" >> results.txt
done
