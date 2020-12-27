# NAME: Steven Chu
# EMAIL: schu92620@gmail.com
# ID: 905094800

default: lab4b.c
	gcc lab4b.c -o lab4b -Wall -Wextra -lmraa -lm

check: test.sh
	chmod 777 test.sh
	./test.sh

dist: lab4b.c test.sh README Makefile
	tar -czvf lab4b-905094800.tar.gz Makefile README lab4b.c test.sh

clean:
	rm -f lab4b-905094800.tar.gz lab4b
	
