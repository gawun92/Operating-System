
default: build

dist:
	tar -czvf lab3a-305186572.tar.gz lab3a.c ext2_fs.h Makefile README
clean:
	rm -f lab3a lab3a-305186572.tar.gz
build:
	gcc -Wall -Wextra -std=gnu99 -o lab3a lab3a.c -lm

#scp:
#	scp * wun@lnxsrv09.seas.ucla.edu:~/TEST

