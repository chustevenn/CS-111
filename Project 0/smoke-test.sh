#!/bin/bash

# NAME: Steven Chu
# EMAIL: schu92620@gmail.com
# ID: 905094800

# Test standard input/output
echo "Testing" > input
./lab0 < input > output
cmp input output
if [ $? -eq 0 ]
then
    echo "Standard I/O case successful."
else
    echo "Standard I/O case failed."
fi

# Test file input to standard output
echo "Testing" > input
./lab0 --input input > output
cmp input output
if [ $? -eq 0 ]
then
    echo "File input case successful."
else
    echo "File input case failed."
fi

# Test standard input to file output
echo "Testing" > input
./lab0 --output output < input
cmp input output
if [ $? -eq 0 ]
then
    echo "File output case successful."
else
    echo "File output case failed."
fi

# Test file input to file output
echo "Testing" > input
./lab0 --input input --output output
cmp input output
if [ $? -eq 0 ]
then
    echo "File I/O case successful."
else
    echo "File I/O case failed."
fi

# Test segmentation fault option
./lab0 --segfault
if [ $? -eq 139 ]
then
    echo "Segmentation fault case successful."
else
    echo "Segmentation fault case failed."
fi

# Test segmentation fault handling
./lab0 --catch --segfault > /dev/null 2>&1
if [ $? -eq 4 ]
then
    echo "Segmentation fault handling successful."
else
    echo "Segmentation fault handling failed."
fi

# Test invalid input file handling
./lab0 --input foo.txt > output > /dev/null 2>&1
if [ $? -eq 2 ]
then
    echo "Invalid input handling successful."
else
    echo "Invalid input handling failed."
fi

# Test invalid output file handling
chmod u-w output
echo "Testing" > input
./lab0 --output output < input > /dev/null 2>&1
if [ $? -eq 3 ]
then
    echo "Invalid output handling successful."
else
    echo "Invalid output handling failed."
fi

# Test invalid argument handling
./lab0 --wowthisisnotanargumentoption input > /dev/null 2>&1
if [ $? -eq 1 ]
then
    echo "Invalid argument handling successful."
else
    echo "Invalid argument handling failed."
fi

rm -f input output
