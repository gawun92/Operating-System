
 
default: build

check: 
	chmod 755 test.sh
	./test.sh

build: lab4b.c 
	gcc -Wall -Wextra -g -lm -lmraa lab4b.c -o lab4b
	
clean: 
	rm -f lab4b-305186572.tar.gz lab4b 

dist: Makefile README lab4b.c test.sh
	tar -czf lab4b-305186572.tar.gz lab4b.c test.sh README Makefile


scp: 
	scp * root@192.168.7.2:
