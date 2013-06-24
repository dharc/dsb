#!/bin/bash

TESTDIR="$1"

if [ "$TESTDIR" == "" ]; then
	TESTDIR="Debug"
fi

cd $TESTDIR

if [ -x "./nid-test" ]; then
	echo "================================================================================"
	echo "  DSB Tests"
	echo "================================================================================"
	./nid-test
	./event-test
	./harc-test
	./router-test
	./module-test
	./volatile-test
	./asm-test
	./vm-test
	./proc-test
	./name-test
	echo "================================================================================"
else
	echo "Cannot find tests!"
fi
