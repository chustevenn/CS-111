# NAME: Steven Chu
# EMAIL: schu92620@gmail.com
# ID: 905094800

default:
	gcc lab4c_tls.c -o lab4c_tls -lmraa -lm -Wall -Wextra -lssl -lcrypto
	gcc lab4c_tcp.c -o lab4c_tcp -lmraa -lm -Wall -Wextra

dist: lab4c_tls.c lab4c_tcp.c README Makefile
	tar -czvf lab4c-905094800.tar.gz lab4c_tls.c lab4c_tcp.c Makefile README

clean:
	rm -f lab4c-905094800.tar.gz lab4c_tls lab4c_tcp
