default: build

build:
	rm -f lab3b
	echo 'python2.7 lab3b.py $$1' >> lab3b
	chmod 755 lab3b

dist:
	tar -czvf lab3b-305186572.tar.gz README Makefile lab3b.py

clean:
	rm -rf lab3b lab3b-305186572.tar.gz

