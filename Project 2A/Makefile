# NAME: Steven Chu
# EMAIL: schu92620@gmail.com
# ID: 905094800

default: build

build: SortedList.h SortedList.c lab2_add.c lab2_list.c
	gcc lab2_add.c -o lab2_add -Wall -Wextra -pthread
	gcc lab2_list.c SortedList.c -o lab2_list -Wall -Wextra -pthread

tests: lab2_add lab2_list test.sh
	chmod 777 test.sh
	./test.sh

graphs: tests lab2_add.gp lab2_list.gp
	gnuplot -c lab2_add.gp lab2_add.csv
	gnuplot -c lab2_list.gp lab2_list.csv

dist: build Makefile README tests graphs
	tar -czvf lab2a-905094800.tar.gz lab2_list.csv lab2_add.csv lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png lab2_add.c lab2_list.c SortedList.h SortedList.c lab2_add.gp lab2_list.gp Makefile README test.sh

clean:
	rm -f lab2_add lab2_list lab2a-905094800.tar.gz
