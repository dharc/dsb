#!/bin/sh

cd $1

echo "================================================================================"
echo "  DSB Tests"
echo "================================================================================"
./nid-test
./event-test
./harc-test
./router-test
./module-test
./volatile-test
./eval-test
./math-test
echo "================================================================================"
