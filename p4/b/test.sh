#!/bin/bash
 
./lab4b --XX
if [[ $? -ne 1 ]]
then
	echo "fail - catch invalid argument"
else
	echo "pass - catch invalid argument"
fi

./lab4b --period=-2
if [[ $? -ne 1 ]]
then
	echo "fail - catch invalid number of period"
else
	echo "pass - catch invalid number of period"
fi

./lab4b --period=2 --scale=F --log=LOG <<-EOF
SCALE=C
PERIOD=3
STOP
START
LOG 
OFF
EOF

if [ ! -s LOG ]
then
        echo "fail - create log file"
else
	echo "pass - create log file"
fi


if [[ $? -ne 0 ]]
then
        echo "fail - return the proper exit value"
else
	echo "pass - return the proper exit value"
fi


