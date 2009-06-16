#!/bin/bash
# Usage: test

### Start main procedure

for n in 1 2 4 8 16 32
do
	for r in 10 50 100 500 1000
	do
		mpprun --nranks=$n ./mpi-filter $r im1.ppm test.ppm > blur_n${n}_${r}.log
	done
done

for n in 1 2 4 8 16 32
do
	mpprun --nranks=$n ./mpi-filter $n im1.ppm test.ppm > threshold_n${n}.log
done
