# NAME: Steven Chu
# EMAIL: schu92620@gmail.com
# ID: 905094800

default:
	rm -f lab3b
	ln -s lab3b.py lab3b
	chmod 777 lab3b
	echo "lab3b executable link created."

dist: lab3b.py README Makefile
	tar -czvf lab3b-905094800.tar.gz lab3b.py Makefile README

clean:
	rm -f lab3b-905094800.tar.gz lab3b
