#!/bin/sh

cd $1
./nid-test
./event-test
./harc-test
./router-test
./module-test
./volatile-test
./eval-test
./math-test

