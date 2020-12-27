# NAME: Steven Chu
# EMAIL: schu92620@gmail.com
# 905094800

default: lab0.c
	gcc lab0.c -o lab0 -Wall -Wextra -g

dist: lab0.c Makefile smoke-test.sh README backtrace.png breakpoint.png
	tar -czvf lab0-905094800.tar.gz lab0.c Makefile smoke-test.sh README backtrace.png breakpoint.png

clean:
	rm -f lab0-905094800.tar.gz lab0

check: smoke-test.sh lab0
	chmod 777 smoke-test.sh
	./smoke-test.sh
