
#Purpose:
#        generate data reduction graphs for the multi-threaded list project
#
# input: lab2b_list.csv
#       1. test name
#       2. # threads
#       3. # iterations per thread
#       4. # lists
#       5. # operations performed (threads x iterations x (ins + lookup + delete))
#       6. run time (ns)
#       7. run time per operation (ns)
#	8. average lock wait time (ns)
#
# output:
#       lab2_list-1.png ... cost per operation vs threads and iterations
#       lab2_list-2.png ... threads and iterations that run (un-protected) w/o failure
#       lab2_list-3.png ... threads and iterations that run (protected) w/o failure
#       lab2_list-4.png ... cost per operation vs number of threads
#
# Note:
#       Managing data is simplified by keeping all of the results in a single
#       file.  But this means that the individual graphing commands have to
#       grep to select only the data they want.
#
#       Early in your implementation, you will not have data for all of the
#       tests, and the later sections may generate errors for missing data.
#

# general plot parameters
set terminal png
set datafile separator ","
billion = 1000000000

# Aggregate throughput of mutex and spin-lock
set title "List-1: Operations per second vs. Number of Threads"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Operations per second"
set logscale y 10
set output 'lab2b_1.png'
set key left top
plot \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(billion/($7)) \
        title 'list w/mutex' with linespoints lc rgb 'purple', \
     "< grep -e 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(billion/($7)) \
        title 'list w/spin-lock' with linespoints lc rgb 'blue'

# Mutex average wait-for-lock time
set title "List-2: Mutex threads vs. Avg wait time and lock time"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Time (ns)"
set logscale y 10
set output 'lab2b_2.png'
set key left top
plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($8) \
	title 'average wait time to lock' with linespoints lc rgb 'purple', \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($7) \
	title 'average per operation time' with linespoints lc rgb 'blue'

set title "List-3: Unprotected Threads and Iterations that run without failure"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Successful Iterations"
set logscale y 10
set output 'lab2b_3.png'
plot \
     "< grep 'list-id-none,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	title 'not synchronized' with points lc rgb 'purple', \
     "< grep 'list-id-m,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	title 'mutex lock' with points lc rgb 'blue', \
     "< grep 'list-id-s,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	title 'spin lock' with points lc rgb 'green'


set title "List-4: Mutex lock performance vs. Number of sublists"
set xlabel "Sublists"
set logscale x 2
set xrange [0.75:]
set ylabel "Operations per second"
set logscale y 10
set output 'lab2b_4.png'
set key left top
plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(billion/($7)) \
	title '1 sublist' with linespoints lc rgb 'purple', \
	 "< grep 'list-none-m,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(billion/($7)) \
	title '4 sublists' with linespoints lc rgb 'blue', \
     "< grep 'list-none-m,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(billion/($7)) \
	title '8 sublists' with linespoints lc rgb 'green', \
	 "< grep 'list-none-m,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(billion/($7)) \
	title '16 sublists' with linespoints lc rgb 'black'


set title "List-5: Spin lock performance vs. Number of sublists"
set xlabel "Sublists"
set logscale x 2
set xrange [0.75:]
set ylabel "Operations per second"
set logscale y 10
set output 'lab2b_5.png'
set key left top
plot \
     "< grep 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(billion/($7)) \
        title '1 sublist' with linespoints lc rgb 'purple', \
         "< grep 'list-none-s,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(billion/($7)) \
        title '4 sublists' with linespoints lc rgb 'blue', \
     "< grep 'list-none-s,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(billion/($7)) \
        title '8 sublists' with linespoints lc rgb 'green', \
         "< grep 'list-none-s,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(billion/($7)) \
        title '16 sublists' with linespoints lc rgb 'black'



