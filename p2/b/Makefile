

build: lab2_list.c SortedList.c SortedList.h
	gcc -Wall -Wextra -Werror -o lab2_list lab2_list.c SortedList.c  -lpthread -lprofiler

graphs: tests
	chmod 755 lab2_list.gp
	gnuplot lab2_list.gp

dist: clean graphs profile lab2_list.c SortedList.c SortedList.h Makefile README 
	tar -czf lab2b-305186572.tar.gz lab2_list.c Makefile README SortedList.c \
		SortedList.h lab2b_*.png lab2b_list.csv lab2_list.gp profile.out

profile: clean build
	rm -f raw.gperf profile.out
	LD_PRELOAD=/usr/lib64/libprofiler.so \
	CPUPROFILE=raw.gperf ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list ./raw.gperf > profile.out
	pprof --list=threadfunc ./lab2_list ./raw.gperf >> profile.out
	-rm -f raw.gperf

clean:	
	rm -rf lab2b-305186572.tar.gz lab2_list  

default: build

tests: clean build
	echo " " > lab2b_list.csv
	./lab2_list --iterations=1000 --sync=s --threads=1 >> lab2b_list.csv
	./lab2_list --iterations=1000 --sync=s --threads=2 >> lab2b_list.csv
	./lab2_list --iterations=1000 --sync=s --threads=4 >> lab2b_list.csv
	./lab2_list --iterations=1000 --sync=s --threads=8 >> lab2b_list.csv
	./lab2_list --iterations=1000 --sync=s --threads=16 >> lab2b_list.csv
	./lab2_list --iterations=1000 --sync=s --threads=32 >> lab2b_list.csv

	./lab2_list --iterations=1000 --sync=m --threads=1 >> lab2b_list.csv
	./lab2_list --iterations=1000 --sync=m --threads=2 >> lab2b_list.csv
	./lab2_list --iterations=1000 --sync=m --threads=4 >> lab2b_list.csv
	./lab2_list --iterations=1000 --sync=m --threads=8 >> lab2b_list.csv
	./lab2_list --iterations=1000 --sync=m --threads=16 >> lab2b_list.csv
	./lab2_list --iterations=1000 --sync=m --threads=32 >> lab2b_list.csv
	
	-./lab2_list --yield=id --lists=4 --threads=1 --iterations=1 >> lab2b_list.csv 
	-./lab2_list --yield=id --lists=4 --threads=1 --iterations=2 >> lab2b_list.csv 
	-./lab2_list --yield=id --lists=4 --threads=1 --iterations=4 >> lab2b_list.csv 
	-./lab2_list --yield=id --lists=4 --threads=1 --iterations=8 >> lab2b_list.csv 
	-./lab2_list --yield=id --lists=4 --threads=1 --iterations=16 >> lab2b_list.csv 
	-./lab2_list --yield=id --lists=4 --threads=1 --iterations=32 >> lab2b_list.csv 

	-./lab2_list --yield=id --lists=4 --threads=4 --iterations=1 >> lab2b_list.csv 
	-./lab2_list --yield=id --lists=4 --threads=4 --iterations=2 >> lab2b_list.csv 
	-./lab2_list --yield=id --lists=4 --threads=4 --iterations=4 >> lab2b_list.csv 
	-./lab2_list --yield=id --lists=4 --threads=4 --iterations=8 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=4 --iterations=16 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=4 --iterations=32 >> lab2b_list.csv

	-./lab2_list --yield=id --lists=4 --threads=8 --iterations=1 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=8 --iterations=2 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=8 --iterations=4 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=8 --iterations=8 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=8 --iterations=16 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=8 --iterations=32 >> lab2b_list.csv

	-./lab2_list --yield=id --lists=4 --threads=12 --iterations=1 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=12 --iterations=2 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=12 --iterations=4 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=12 --iterations=8 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=12 --iterations=16 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=12 --iterations=32 >> lab2b_list.csv

	-./lab2_list --yield=id --lists=4 --threads=16 --iterations=1 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=16 --iterations=2 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=16 --iterations=4 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=16 --iterations=8 >> lab2b_list.csv 
	-./lab2_list --yield=id --lists=4 --threads=16 --iterations=16 >> lab2b_list.csv 
	-./lab2_list --yield=id --lists=4 --threads=16 --iterations=32 >> lab2b_list.csv

	-./lab2_list --yield=id --lists=4 --threads=32 --iterations=1 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=32 --iterations=2 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=32 --iterations=4 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=32 --iterations=8 >> lab2b_list.csv 
	-./lab2_list --yield=id --lists=4 --threads=32 --iterations=16 >> lab2b_list.csv 
	-./lab2_list --yield=id --lists=4 --threads=32 --iterations=32 >> lab2b_list.csv

	./lab2_list --yield=id --lists=4 --threads=1 --iterations=10 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=1 --iterations=20 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=1 --iterations=40 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=1 --iterations=80 --sync=s >> lab2b_list.csv

	./lab2_list --yield=id --lists=4 --threads=4 --iterations=10 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=4 --iterations=20 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=4 --iterations=40 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=4 --iterations=80 --sync=s >> lab2b_list.csv

	./lab2_list --yield=id --lists=4 --threads=8 --iterations=10 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=8 --iterations=20 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=8 --iterations=40 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=8 --iterations=80 --sync=s >> lab2b_list.csv

	./lab2_list --yield=id --lists=4 --threads=12 --iterations=10 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=12 --iterations=20 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=12 --iterations=40 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=12 --iterations=80 --sync=s >> lab2b_list.csv

	./lab2_list --yield=id --lists=4 --threads=16 --iterations=10 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=16 --iterations=20 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=16 --iterations=40 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=16 --iterations=80 --sync=s >> lab2b_list.csv

	./lab2_list --yield=id --lists=4 --threads=1 --iterations=10 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=1 --iterations=20 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=1 --iterations=40 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=4 --iterations=10 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=4 --iterations=20 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=4 --iterations=40 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=4 --iterations=80 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=8 --iterations=10 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=8 --iterations=20 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=8 --iterations=40 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=8 --iterations=80 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=12 --iterations=10 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=12 --iterations=20 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=12 --iterations=40 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=12 --iterations=80 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=16 --iterations=10 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=16 --iterations=20 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=16 --iterations=40 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=16 --iterations=80 --sync=m >> lab2b_list.csv

	./lab2_list --sync=s --iterations=1000 --threads=1 --lists=4 >> lab2b_list.csv
	./lab2_list --sync=s --iterations=1000 --threads=1 --lists=8 >> lab2b_list.csv
	./lab2_list --sync=s --iterations=1000 --threads=1 --lists=16 >> lab2b_list.csv
	./lab2_list --sync=s --iterations=1000 --threads=2 --lists=4 >> lab2b_list.csv
	./lab2_list --sync=s --iterations=1000 --threads=2 --lists=8 >> lab2b_list.csv
	./lab2_list --sync=s --iterations=1000 --threads=2 --lists=16 >> lab2b_list.csv
	./lab2_list --sync=s --iterations=1000 --threads=4 --lists=4 >> lab2b_list.csv
	./lab2_list --sync=s --iterations=1000 --threads=4 --lists=8 >> lab2b_list.csv
	./lab2_list --sync=s --iterations=1000 --threads=4 --lists=16 >> lab2b_list.csv
	./lab2_list --sync=s --iterations=1000 --threads=8 --lists=4 >> lab2b_list.csv
	./lab2_list --sync=s --iterations=1000 --threads=8 --lists=8 >> lab2b_list.csv
	./lab2_list --sync=s --iterations=1000 --threads=8 --lists=16 >> lab2b_list.csv
	./lab2_list --sync=s --iterations=1000 --threads=12 --lists=4 >> lab2b_list.csv
	./lab2_list --sync=s --iterations=1000 --threads=12 --lists=8 >> lab2b_list.csv
	./lab2_list --sync=s --iterations=1000 --threads=12 --lists=16 >> lab2b_list.csv

	./lab2_list --sync=m --iterations=1000 --threads=1 --lists=4 >> lab2b_list.csv
	./lab2_list --sync=m --iterations=1000 --threads=1 --lists=8 >> lab2b_list.csv
	./lab2_list --sync=m --iterations=1000 --threads=1 --lists=16 >> lab2b_list.csv
	./lab2_list --sync=m --iterations=1000 --threads=2 --lists=4 >> lab2b_list.csv
	./lab2_list --sync=m --iterations=1000 --threads=2 --lists=8 >> lab2b_list.csv
	./lab2_list --sync=m --iterations=1000 --threads=2 --lists=16 >> lab2b_list.csv
	./lab2_list --sync=m --iterations=1000 --threads=4 --lists=4 >> lab2b_list.csv
	./lab2_list --sync=m --iterations=1000 --threads=4 --lists=8 >> lab2b_list.csv
	./lab2_list --sync=m --iterations=1000 --threads=4 --lists=16 >> lab2b_list.csv
	./lab2_list --sync=m --iterations=1000 --threads=8 --lists=4 >> lab2b_list.csv
	./lab2_list --sync=m --iterations=1000 --threads=8 --lists=8 >> lab2b_list.csv
	./lab2_list --sync=m --iterations=1000 --threads=8 --lists=16 >> lab2b_list.csv
	./lab2_list --sync=m --iterations=1000 --threads=12 --lists=4 >> lab2b_list.csv
	./lab2_list --sync=m --iterations=1000 --threads=12 --lists=8 >> lab2b_list.csv
	./lab2_list --sync=m --iterations=1000 --threads=12 --lists=16 >> lab2b_list.csv
