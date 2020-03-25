#!/bin/env bash

for K in 200 400 800 1000 2000 4000 8000 16000 32000 64000; do
	for threads in 2 4 8 16 32 64 128; do
		./my_list-forming $threads $K
	done
done

for K in 200 400 800 1000 2000 4000 8000 16000 32000 64000; do
	for threads in 2 4 8 16 32 64 128; do
		./list-forming $threads $K
	done
done

echo "Done, bitch"
