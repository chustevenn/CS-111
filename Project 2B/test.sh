#!/bin/bash

rm -f lab2b_list.csv
# Test mutex and spin lock list operations: 1,000 iterations, 1,2,4,8,12,16,24 threads
for i in m s
do
	for j in 1 2 4 8 12 16 24
	do
		./lab2_list --threads=$j --iterations=1000 --sync=$i >> lab2b_list.csv
	done
done

# Test with --yield=id, 4 lists, 1,4,8,12,16 threads, and 1,2,4,8,16 iterations
for i in 1 4 8 12 16
do
	for j in 1 2 4 8 16
	do	
		./lab2_list --threads=$i --iterations=$j --yield=id --lists=4 >> lab2b_list.csv
	done
done

# Test with synchronization and 10,20,40,80 iterations
for k in m s
do
	for i in 1 4 8 12 16
	do
		for j in 10 20 40 80
		do		
			./lab2_list --threads=$i --iterations=$j --yield=id --lists=4 --sync=$k >> lab2b_list.csv
		done
	done
done

# Test again with no yields, both synchronized versions, 1000 iterations, 1,2,4,8,12 threads
# 1,4,8,16 lists
for i in m s
do
	for j in 1 2 4 8 12
	do
		for k in 4 8 16
		do
			./lab2_list --threads=$j --iterations=1000 --lists=$k --sync=$i  >> lab2b_list.csv
		done
	done
done
