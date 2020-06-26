#! /usr/bin/gnuplot
# purpose:
#    generate data reduction graphs for the multi-threaded list project
#
# input: lab2_list.csv
#   1. test name
#   2. # threads
#   3. # iterations per thread
#   4. # lists
#   5. # operations performed (threads x iterations x (ins + lookup + delete))
#   6. run time (ns)
#   7. run time per operation (ns)
#
# output:
# lab2b_1.png ... throughput vs. number of threads for mutex and spin-lock synchronized list operations.
# lab2b_2.png ... mean time per mutex wait and mean time per operation for mutex-synchronized list operations.
# lab2b_3.png ... successful iterations vs. threads for each synchronization method.
#lab2b_4.png ... throughput vs. number of threads for mutex synchronized partitioned lists.
#lab2b_5.png ... throughput vs. number of threads for spin-lock-synchronized partitioned lists.
#
# Note:
#    Managing data is simplified by keeping all of the results in a single
#    file.  But this means that the individual graphing commands have to
#    grep to select only the data they want.
#
#    Early in your implementation, you will not have data for all of the
#    tests, and the later sections may generate errors for missing data.
#

# general plot parameters
set terminal png
set datafile separator ","

# throughput(operations/sec) vs.number of threads (mutex and spin lock)
set title "List-1: Throughput vs Number of Threads"
set xlabel "Number of Threads"
set logscale x 2
set xrange [1:32]
set ylabel "Throughput(operations/sec)"
set logscale y 10
set output 'lab2b_1.png'
set key right top

plot \
    "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
    title 'Mutex' with linespoints lc rgb 'red', \
    "< grep -e 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
    title 'Spin Lock' with linespoints lc rgb 'blue'




set title "List-2: Mean time per mutex wait and operation for mutex-synchronized list operations."
set xlabel "Number of Threads"
set logscale x 2
set xrange [1:32]
set ylabel "Time"
set logscale y 10
set output 'lab2b_2.png'
set key left top

plot \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($7) \
    title 'Completion Time' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($8) \
    title 'Wait Time' with linespoints lc rgb 'red'
     


set title "List-3: Successful iterations vs. threads for each synchronization method"
set logscale x 2
set xrange [1:32]
set xrange [0.5:]
set xlabel "Threads"
set ylabel "Successful Iterations"
set logscale y 10
set key right top
set output 'lab2b_3.png'

plot \
    "< grep -e 'list-id-none,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
    with points lc rgb 'red' title 'unprotected', \
    "< grep -e 'list-id-m,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
    with points lc rgb 'blue' title 'Mutex', \
    "< grep -e 'list-id-s,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
    with points lc rgb 'green' title 'Spin Lock', \




set title "List-4: Throughput vs. number of threads for mutex synchronized partitioned lists."
set xlabel "Number of Threads"
set logscale x 2
set xrange [0.5:]
set ylabel "Throughput"
set logscale y 10
set output 'lab2b_4.png'
set key right top

plot \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
     with linespoints lc rgb 'yellow' title '1 Sublist', \
     "< grep -e 'list-none-m,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
     with linespoints lc rgb 'green' title '4 Sublist', \
     "< grep -e 'list-none-m,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
     with linespoints lc rgb 'red' title '8 Sublist', \
     "< grep -e 'list-none-m,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
     with linespoints lc rgb 'blue' title '16 Sublist'

set title "List-5: Throughput vs. number of threads for spin-lock-synchronized partitioned lists."
set xlabel "Number of Threads"
set logscale x 2
set xrange [0.5:]
set ylabel "Throughput"
set logscale y 10
set output 'lab2b_5.png'
set key right top

plot \
     "< grep -e 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
     with linespoints lc rgb 'yellow' title '1 Sublist', \
     "< grep -e 'list-none-s,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
     with linespoints lc rgb 'green' title '4 Sublist', \
     "< grep -e 'list-none-s,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
     with linespoints lc rgb 'red' title '8 Sublist', \
     "< grep -e 'list-none-s,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
     with linespoints lc rgb 'blue' title '16 Sublist'
