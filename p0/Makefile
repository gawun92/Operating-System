CC = gcc
 
default: 
	$(CC) -Wall -Wextra -g -o lab0 lab0.c
all:
	$(CC) -Wall -Wextra -g -o lab0 lab0.c
dist:
	tar -cvzf lab0-305186572.tar.gz lab0.c Makefile README backtrace.png breakpoint.png
check: all
	./lab0 --input=zzzzzzzzzzzzzzzzzzzzzzzzzz.txt ;\
	if [ $$? -eq 2 ] ; \
	then\
		./lab0 --output= ;\
		if [ $$? -eq 3 ] ;\
		then\
			./lab0 --asdf ;\
			if [ $$? -eq 1 ] ;\
			then\
				echo "----------------------------------------------TEST CASE PASSED" ;\
			fi ;\
		fi ;\
	fi;\
	
clean:
	rm -f lab0 *.txt *.o *.tar.gz
