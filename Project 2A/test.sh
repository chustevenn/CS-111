#!/bin/bash

# Testing threads and iterations to cause a failure
./lab2_add --threads=2  --iterations=100    >  lab2_add.csv
./lab2_add --threads=2  --iterations=1000   >> lab2_add.csv
./lab2_add --threads=2  --iterations=10000  >> lab2_add.csv
./lab2_add --threads=2  --iterations=100000 >> lab2_add.csv
./lab2_add --threads=4  --iterations=100    >> lab2_add.csv
./lab2_add --threads=4  --iterations=1000   >> lab2_add.csv
./lab2_add --threads=4  --iterations=10000  >> lab2_add.csv
./lab2_add --threads=4  --iterations=100000 >> lab2_add.csv
./lab2_add --threads=8  --iterations=100    >> lab2_add.csv
./lab2_add --threads=8  --iterations=1000   >> lab2_add.csv
./lab2_add --threads=8  --iterations=10000  >> lab2_add.csv
./lab2_add --threads=8  --iterations=100000 >> lab2_add.csv
./lab2_add --threads=12 --iterations=100    >> lab2_add.csv
./lab2_add --threads=12 --iterations=1000   >> lab2_add.csv
./lab2_add --threads=12 --iterations=10000  >> lab2_add.csv
./lab2_add --threads=12 --iterations=100000  >> lab2_add.csv
# Number of iterations and threads with yield to cause failure
./lab2_add --threads=2  --iterations=10     --yield >> lab2_add.csv
./lab2_add --threads=4  --iterations=10     --yield >> lab2_add.csv
./lab2_add --threads=8  --iterations=10     --yield >> lab2_add.csv
./lab2_add --threads=12 --iterations=10     --yield >> lab2_add.csv
./lab2_add --threads=2  --iterations=20     --yield >> lab2_add.csv
./lab2_add --threads=4  --iterations=20     --yield >> lab2_add.csv
./lab2_add --threads=8  --iterations=20     --yield >> lab2_add.csv
./lab2_add --threads=12 --iterations=20     --yield >> lab2_add.csv
./lab2_add --threads=2  --iterations=40     --yield >> lab2_add.csv
./lab2_add --threads=4  --iterations=40     --yield >> lab2_add.csv
./lab2_add --threads=8  --iterations=40     --yield >> lab2_add.csv
./lab2_add --threads=12 --iterations=40     --yield >> lab2_add.csv
./lab2_add --threads=2  --iterations=80     --yield >> lab2_add.csv
./lab2_add --threads=4  --iterations=80     --yield >> lab2_add.csv
./lab2_add --threads=8  --iterations=80     --yield >> lab2_add.csv
./lab2_add --threads=12 --iterations=80     --yield >> lab2_add.csv
./lab2_add --threads=2  --iterations=100    --yield >> lab2_add.csv
./lab2_add --threads=4  --iterations=100    --yield >> lab2_add.csv
./lab2_add --threads=8  --iterations=100    --yield >> lab2_add.csv
./lab2_add --threads=12 --iterations=100    --yield >> lab2_add.csv
./lab2_add --threads=2  --iterations=1000   --yield >> lab2_add.csv
./lab2_add --threads=4  --iterations=1000   --yield >> lab2_add.csv
./lab2_add --threads=8  --iterations=1000   --yield >> lab2_add.csv
./lab2_add --threads=12 --iterations=1000   --yield >> lab2_add.csv
./lab2_add --threads=2  --iterations=10000  --yield >> lab2_add.csv
./lab2_add --threads=4  --iterations=10000  --yield >> lab2_add.csv
./lab2_add --threads=8  --iterations=10000  --yield >> lab2_add.csv
./lab2_add --threads=12 --iterations=10000  --yield >> lab2_add.csv
./lab2_add --threads=2  --iterations=100000 --yield >> lab2_add.csv
./lab2_add --threads=4  --iterations=100000 --yield >> lab2_add.csv
./lab2_add --threads=8  --iterations=100000 --yield >> lab2_add.csv
./lab2_add --threads=12 --iterations=100000 --yield >> lab2_add.csv
# Average cost per operation vs. number of iterations
./lab2_add --threads=1  --iterations=100         >> lab2_add.csv
./lab2_add --threads=1  --iterations=1000        >> lab2_add.csv
./lab2_add --threads=1  --iterations=10000       >> lab2_add.csv
./lab2_add --threads=1  --iterations=100000      >> lab2_add.csv
./lab2_add --threads=1  --iterations=1000000     >> lab2_add.csv
./lab2_add --threads=2  --iterations=100         >> lab2_add.csv
./lab2_add --threads=2  --iterations=1000        >> lab2_add.csv
./lab2_add --threads=2  --iterations=10000       >> lab2_add.csv
./lab2_add --threads=2  --iterations=100000      >> lab2_add.csv
./lab2_add --threads=2  --iterations=1000000     >> lab2_add.csv
./lab2_add --threads=4  --iterations=100         >> lab2_add.csv
./lab2_add --threads=4  --iterations=1000        >> lab2_add.csv
./lab2_add --threads=4  --iterations=10000       >> lab2_add.csv
./lab2_add --threads=4  --iterations=100000      >> lab2_add.csv
./lab2_add --threads=4  --iterations=1000000     >> lab2_add.csv
./lab2_add --threads=1  --iterations=100     --yield >> lab2_add.csv
./lab2_add --threads=1  --iterations=1000    --yield >> lab2_add.csv
./lab2_add --threads=1  --iterations=10000   --yield >> lab2_add.csv
./lab2_add --threads=1  --iterations=100000  --yield >> lab2_add.csv
./lab2_add --threads=1  --iterations=1000000 --yield >> lab2_add.csv
./lab2_add --threads=2  --iterations=100     --yield >> lab2_add.csv
./lab2_add --threads=2  --iterations=1000    --yield >> lab2_add.csv
./lab2_add --threads=2  --iterations=10000   --yield >> lab2_add.csv
./lab2_add --threads=2  --iterations=100000  --yield >> lab2_add.csv
./lab2_add --threads=2  --iterations=1000000 --yield >> lab2_add.csv
./lab2_add --threads=4  --iterations=100     --yield >> lab2_add.csv
./lab2_add --threads=4  --iterations=1000    --yield >> lab2_add.csv
./lab2_add --threads=4  --iterations=10000   --yield >> lab2_add.csv
./lab2_add --threads=4  --iterations=100000  --yield >> lab2_add.csv
./lab2_add --threads=4  --iterations=1000000 --yield >> lab2_add.csv
# Demonstrate the effectiveness of synchronization mechanisms
./lab2_add --threads=2  --iterations=10000 --yield --sync=m >> lab2_add.csv
./lab2_add --threads=4  --iterations=10000 --yield --sync=m >> lab2_add.csv
./lab2_add --threads=8  --iterations=10000 --yield --sync=m >> lab2_add.csv
./lab2_add --threads=12 --iterations=10000 --yield --sync=m >> lab2_add.csv
./lab2_add --threads=2  --iterations=10000 --yield --sync=c >> lab2_add.csv
./lab2_add --threads=4  --iterations=10000 --yield --sync=c >> lab2_add.csv
./lab2_add --threads=8  --iterations=10000 --yield --sync=c >> lab2_add.csv
./lab2_add --threads=12 --iterations=10000 --yield --sync=c >> lab2_add.csv
./lab2_add --threads=2  --iterations=1000  --yield --sync=s >> lab2_add.csv
./lab2_add --threads=4  --iterations=1000  --yield --sync=s >> lab2_add.csv
./lab2_add --threads=8  --iterations=1000  --yield --sync=s >> lab2_add.csv
./lab2_add --threads=12 --iterations=1000  --yield --sync=s >> lab2_add.csv
# Average time of non-yield operations vs. number of threads
./lab2_add --threads=1  --iterations=10000 --sync=m >> lab2_add.csv
./lab2_add --threads=2  --iterations=10000 --sync=m >> lab2_add.csv
./lab2_add --threads=4  --iterations=10000 --sync=m >> lab2_add.csv
./lab2_add --threads=8  --iterations=10000 --sync=m >> lab2_add.csv
./lab2_add --threads=12 --iterations=10000 --sync=m >> lab2_add.csv
./lab2_add --threads=1  --iterations=10000 --sync=c >> lab2_add.csv
./lab2_add --threads=2  --iterations=10000 --sync=c >> lab2_add.csv
./lab2_add --threads=4  --iterations=10000 --sync=c >> lab2_add.csv
./lab2_add --threads=8  --iterations=10000 --sync=c >> lab2_add.csv
./lab2_add --threads=12 --iterations=10000 --sync=c >> lab2_add.csv
./lab2_add --threads=1  --iterations=10000 --sync=s >> lab2_add.csv
./lab2_add --threads=2  --iterations=10000 --sync=s >> lab2_add.csv
./lab2_add --threads=4  --iterations=10000 --sync=s >> lab2_add.csv
./lab2_add --threads=8  --iterations=10000 --sync=s >> lab2_add.csv
./lab2_add --threads=12 --iterations=10000 --sync=s >> lab2_add.csv
# Per-operation cost vs. number of iterations
./lab2_list --threads=1  --iterations=10	      >  lab2_list.csv
./lab2_list --threads=1  --iterations=100	      >> lab2_list.csv
./lab2_list --threads=1  --iterations=1000	      >> lab2_list.csv
./lab2_list --threads=1  --iterations=10000	      >> lab2_list.csv
./lab2_list --threads=1  --iterations=20000	      >> lab2_list.csv
# Demonstrate that multithreading causes problems
./lab2_list --threads=2  --iterations=10             >> lab2_list.csv
./lab2_list --threads=2  --iterations=100            >> lab2_list.csv
./lab2_list --threads=2  --iterations=1000           >> lab2_list.csv
./lab2_list --threads=4  --iterations=10             >> lab2_list.csv
./lab2_list --threads=4  --iterations=100            >> lab2_list.csv
./lab2_list --threads=4  --iterations=1000           >> lab2_list.csv
./lab2_list --threads=8  --iterations=10             >> lab2_list.csv
./lab2_list --threads=8  --iterations=100            >> lab2_list.csv
./lab2_list --threads=8  --iterations=1000           >> lab2_list.csv
./lab2_list --threads=12 --iterations=10             >> lab2_list.csv
./lab2_list --threads=12 --iterations=100            >> lab2_list.csv
./lab2_list --threads=12 --iterations=1000           >> lab2_list.csv
# Test number of threads/iterations with different yield combinations that cause problems
./lab2_list --threads=2  --iterations=2   --yield=i  >> lab2_list.csv
./lab2_list --threads=2  --iterations=4   --yield=i  >> lab2_list.csv
./lab2_list --threads=2  --iterations=8   --yield=i  >> lab2_list.csv
./lab2_list --threads=2  --iterations=16  --yield=i  >> lab2_list.csv
./lab2_list --threads=2  --iterations=32  --yield=i  >> lab2_list.csv
./lab2_list --threads=4  --iterations=2   --yield=i  >> lab2_list.csv
./lab2_list --threads=4  --iterations=4   --yield=i  >> lab2_list.csv
./lab2_list --threads=4  --iterations=8   --yield=i  >> lab2_list.csv
./lab2_list --threads=4  --iterations=16  --yield=i  >> lab2_list.csv
./lab2_list --threads=8  --iterations=2   --yield=i  >> lab2_list.csv
./lab2_list --threads=8  --iterations=4   --yield=i  >> lab2_list.csv
./lab2_list --threads=8  --iterations=8   --yield=i  >> lab2_list.csv
./lab2_list --threads=8  --iterations=16  --yield=i  >> lab2_list.csv
./lab2_list --threads=12 --iterations=2   --yield=i  >> lab2_list.csv
./lab2_list --threads=12 --iterations=4   --yield=i  >> lab2_list.csv
./lab2_list --threads=12 --iterations=8   --yield=i  >> lab2_list.csv
./lab2_list --threads=12 --iterations=16  --yield=i  >> lab2_list.csv
./lab2_list --threads=2  --iterations=2   --yield=d  >> lab2_list.csv
./lab2_list --threads=2  --iterations=4   --yield=d  >> lab2_list.csv
./lab2_list --threads=2  --iterations=8   --yield=d  >> lab2_list.csv
./lab2_list --threads=2  --iterations=16  --yield=d  >> lab2_list.csv
./lab2_list --threads=2  --iterations=32  --yield=d  >> lab2_list.csv
./lab2_list --threads=4  --iterations=2   --yield=d  >> lab2_list.csv
./lab2_list --threads=4  --iterations=4   --yield=d  >> lab2_list.csv
./lab2_list --threads=4  --iterations=8   --yield=d  >> lab2_list.csv
./lab2_list --threads=4  --iterations=16  --yield=d  >> lab2_list.csv
./lab2_list --threads=8  --iterations=2   --yield=d  >> lab2_list.csv
./lab2_list --threads=8  --iterations=4   --yield=d  >> lab2_list.csv
./lab2_list --threads=8  --iterations=8   --yield=d  >> lab2_list.csv
./lab2_list --threads=8  --iterations=16  --yield=d  >> lab2_list.csv
./lab2_list --threads=12 --iterations=2   --yield=d  >> lab2_list.csv
./lab2_list --threads=12 --iterations=4   --yield=d  >> lab2_list.csv
./lab2_list --threads=12 --iterations=8   --yield=d  >> lab2_list.csv
./lab2_list --threads=12 --iterations=16  --yield=d  >> lab2_list.csv
./lab2_list --threads=2  --iterations=2   --yield=il >> lab2_list.csv
./lab2_list --threads=2  --iterations=4   --yield=il >> lab2_list.csv
./lab2_list --threads=2  --iterations=8   --yield=il >> lab2_list.csv
./lab2_list --threads=2  --iterations=16  --yield=il >> lab2_list.csv
./lab2_list --threads=2  --iterations=32  --yield=il >> lab2_list.csv
./lab2_list --threads=4  --iterations=2   --yield=il >> lab2_list.csv
./lab2_list --threads=4  --iterations=4   --yield=il >> lab2_list.csv
./lab2_list --threads=4  --iterations=8   --yield=il >> lab2_list.csv
./lab2_list --threads=4  --iterations=16  --yield=il >> lab2_list.csv
./lab2_list --threads=8  --iterations=2   --yield=il >> lab2_list.csv
./lab2_list --threads=8  --iterations=4   --yield=il >> lab2_list.csv
./lab2_list --threads=8  --iterations=8   --yield=il >> lab2_list.csv
./lab2_list --threads=8  --iterations=16  --yield=il >> lab2_list.csv
./lab2_list --threads=12 --iterations=2   --yield=il >> lab2_list.csv
./lab2_list --threads=12 --iterations=4   --yield=il >> lab2_list.csv
./lab2_list --threads=12 --iterations=8   --yield=il >> lab2_list.csv
./lab2_list --threads=12 --iterations=16  --yield=il >> lab2_list.csv
./lab2_list --threads=2  --iterations=2   --yield=dl >> lab2_list.csv
./lab2_list --threads=2  --iterations=4   --yield=dl >> lab2_list.csv
./lab2_list --threads=2  --iterations=8   --yield=dl >> lab2_list.csv
./lab2_list --threads=2  --iterations=16  --yield=dl >> lab2_list.csv
./lab2_list --threads=2  --iterations=32  --yield=dl >> lab2_list.csv
./lab2_list --threads=4  --iterations=2   --yield=dl >> lab2_list.csv
./lab2_list --threads=4  --iterations=4   --yield=dl >> lab2_list.csv
./lab2_list --threads=4  --iterations=8   --yield=dl >> lab2_list.csv
./lab2_list --threads=4  --iterations=16  --yield=dl >> lab2_list.csv
./lab2_list --threads=8  --iterations=2   --yield=dl >> lab2_list.csv
./lab2_list --threads=8  --iterations=4   --yield=dl >> lab2_list.csv
./lab2_list --threads=8  --iterations=8   --yield=dl >> lab2_list.csv
./lab2_list --threads=8  --iterations=16  --yield=dl >> lab2_list.csv
./lab2_list --threads=12 --iterations=2   --yield=dl >> lab2_list.csv
./lab2_list --threads=12 --iterations=4   --yield=dl >> lab2_list.csv
./lab2_list --threads=12 --iterations=8   --yield=dl >> lab2_list.csv
./lab2_list --threads=12 --iterations=16  --yield=dl >> lab2_list.csv
# Show that synchronization mechanisms fix problems
./lab2_list --threads=12 --iterations=32 --yield=i  --sync=m >> lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=d  --sync=m >> lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=il --sync=m >> lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=dl --sync=m >> lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=i  --sync=s >> lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=d  --sync=s >> lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=il --sync=s >> lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=dl --sync=s >> lab2_list.csv
# measure performance of mutex and spin
./lab2_list --threads=1  --iterations=1000 --sync=m >> lab2_list.csv
./lab2_list --threads=2  --iterations=1000 --sync=m >> lab2_list.csv
./lab2_list --threads=4  --iterations=1000 --sync=m >> lab2_list.csv
./lab2_list --threads=8  --iterations=1000 --sync=m >> lab2_list.csv
./lab2_list --threads=12 --iterations=1000 --sync=m >> lab2_list.csv
./lab2_list --threads=16 --iterations=1000 --sync=m >> lab2_list.csv
./lab2_list --threads=24 --iterations=1000 --sync=m >> lab2_list.csv
./lab2_list --threads=1  --iterations=1000 --sync=s >> lab2_list.csv
./lab2_list --threads=2  --iterations=1000 --sync=s >> lab2_list.csv
./lab2_list --threads=4  --iterations=1000 --sync=s >> lab2_list.csv
./lab2_list --threads=8  --iterations=1000 --sync=s >> lab2_list.csv
./lab2_list --threads=12 --iterations=1000 --sync=s >> lab2_list.csv
./lab2_list --threads=16 --iterations=1000 --sync=s >> lab2_list.csv
./lab2_list --threads=24 --iterations=1000 --sync=s >> lab2_list.csv
