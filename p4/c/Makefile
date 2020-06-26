#NAME: Gawun Kim
#EMAIL: gawun92@g.ucla.edu
#ID : 305186572

default:
	gcc -g -Wall -Wextra  -lm -lmraa lab4c_tcp.c -o lab4c_tcp
	gcc -g -Wall -Wextra  -lm -lmraa -lcrypto -lssl lab4c_tls.c -o lab4c_tls

clean:
	rm -rf lab4c_tcp lab4c_tls *.o lab4c-305186572.tar.gz

dist: default
	tar -cvzf lab4c-305186572.tar.gz lab4c_tls.c lab4c_tcp.c README Makefile

scp:
	scp * root@192.168.7.2:~/TEST

