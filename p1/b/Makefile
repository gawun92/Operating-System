# NAME: Gawun Kim
# EMAIL: gawun92@g.ucla.edu
# ID: 305186572


default: lab1b-client.c lab1b-server.c 
	gcc -Wall -Wextra lab1b-client.c  -o lab1b-client -lmcrypt
	gcc -Wall -Wextra lab1b-server.c  -o lab1b-server -lmcrypt
clean:
	rm -rf lab1b-client lab1b-server *tar* *~ *.dSYM *.o *.so 

dist:
	tar -czf lab1b-305186572.tar.gz lab1b-client.c lab1b-server.c Makefile README my.key
