# NAME: Steven Chu
# EMAIL: schu92620@gmail.com
# ID: 905094800

default:
	gcc lab2_list.c SortedList.c -o lab2_list -Wall -Wextra -pthread -g

profile: lab2_list
	rm -f raw.gperf profile.out
	LD_PRELOAD=/u/eng/class/classchu/lib/libprofiler.so.0 CPUPROFILE=./raw.gperf ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text lab2_list 			  ./raw.gperf > profile.out
	pprof --list=thread_action 	./lab2_list ./raw.gperf >> profile.out
	pprof --list=list_insert_all 	./lab2_list ./raw.gperf >> profile.out
	pprof --list=list_getlength 	./lab2_list ./raw.gperf >> profile.out
	pprof --list=list_delete 	./lab2_list ./raw.gperf >> profile.out
	pprof --list=spin_lock_timed 	./lab2_list ./raw.gperf >> profile.out

tests: lab2_list test.sh
	chmod 777 test.sh
	./test.sh

graphs: lab2_list.gp tests
	gnuplot -c lab2_list.gp lab2b_list.csv

dist: Makefile README default graphs
	tar -czvf lab2b-905094800.tar.gz lab2_list.c SortedList.h SortedList.c lab2_list.gp lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png lab2b_list.csv profile.out test.sh Makefile README

clean:
	rm -f lab2_list lab2b-905094800.tar.gz raw.gperf
