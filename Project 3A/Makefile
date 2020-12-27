# NAME: Steven Chu
# EMAIL: schu92620@gmail.com
# ID: 905094800

default: lab3a.c
	gcc lab3a.c -o lab3a -Wall -Wextra

dist: lab3a.c Makefile README trivial.csv trivial.img ext2_fs.h
	tar -czvf lab3a-905094800.tar.gz lab3a.c Makefile README trivial.csv trivial.img ext2_fs.h 

clean:
	rm -f lab3a-905094800.tar.gz lab3a
