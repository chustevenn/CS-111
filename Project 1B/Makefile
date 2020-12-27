# NAME: Steven Chu
# EMAIL: schu92620@gmail.com
# ID: 905094800

default: lab1b-server.c lab1b-client.c
	gcc lab1b-server.c -o lab1b-server -Wall -Wextra -lz
	gcc lab1b-client.c -o lab1b-client -Wall -Wextra -lz

client: lab1b-client.c
	gcc lab1b-client.c -o lab1b-client -Wall -Wextra -lz

server: lab1b-server.c
	gcc lab1b-server.c -o lab1b-server -Wall -Wextra -lz

dist: lab1b-server.c lab1b-client.c Makefile README
	tar -czvf lab1b-905094800.tar.gz lab1b-server.c lab1b-client.c Makefile README

clean:
	rm -f lab1b-905094800.tar.gz lab1b-server lab1b-client
