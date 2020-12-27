#!/bin/bash

# Check if --log argument works
./lab4b --log=LOG > STDOUT <<-EOF
OFF
EOF

if [ -s LOG ]
then
	echo "--log successful"
else
	echo "--log failed"
fi

# Check if SCALE=F is processed
./lab4b --log=LOG > STDOUT <<-EOF
SCALE=F
OFF
EOF

grep "SCALE=F" LOG > /dev/null
if [ $? -eq 0 ]
then
	echo "SCALE=F successful"
else
	echo "SCALE=F failed"
fi

# Check if SCALE=C is processed
./lab4b --log=LOG > STDOUT <<-EOF
SCALE=C
OFF
EOF

grep "SCALE=C" LOG > /dev/null
if [ $? -eq 0 ]
then
        echo "SCALE=C successful"
else
        echo "SCALE=C failed"
fi

# Check if SHUTDOWN is output
grep "SHUTDOWN" LOG > /dev/null
if [ $? -eq 0 ]
then
	echo "SHUTDOWN successful"
else
	echo "SHUTDOWN failed"
fi

# Check if START/STOP/OFF are processed
./lab4b --log=LOG > STDOUT <<-EOF
STOP
START
OFF
EOF

grep "START" LOG > /dev/null
if [ $? -eq 0 ]
then
        echo "START successful"
else
        echo "START failed"
fi

grep "STOP" LOG > /dev/null
if [ $? -eq 0 ]
then
        echo "STOP successful"
else
        echo "STOP failed"
fi

grep "OFF" LOG > /dev/null
if [ $? -eq 0 ]
then
        echo "OFF successful"
else
        echo "OFF failed"
fi

# Check if temperature is logged correctly
./lab4b --log=LOG > STDOUT <<-EOF
OFF
EOF
egrep '[0-9][0-9]:[0-9][0-9]:[0-9][0-9]' LOG > /dev/null
if [ $? -eq 0 ]
then
        echo "Temperature read successful"
else
        echo "Temperature read failed"
fi

rm -f LOG
