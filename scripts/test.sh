#!/bin/bash
TMP=`mktemp -d -p .`
SRC=`pwd`
cd $TMP
mkdir input output
../bin/ddfs_server -f input &
SERVER=$?
sleep 3 #Give the server some time to start..
../bin/ddfs -s 127.0.0.1 -f output &
CLIENT=$?
dd if=/dev/urandom of=input/testfile bs=1 count=1024
md5sum input/testfile
sleep 1 #Just to be sure..
md5sum output/input/testfile
cd $SRC
rm -rfv $TMP
kill $CLIENT
kill $SERVER
