#!/bin/bash

#make clean
protoc --cpp_out=. world_ups.proto
protoc --cpp_out=. ups.proto

make clean
make
./main vcm-14733.vm.duke.edu 12345 vcm-14733.vm.duke.edu 6666
