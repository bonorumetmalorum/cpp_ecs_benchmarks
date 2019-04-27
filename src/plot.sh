#!/bin/bash

sudo sh -c 'echo -1 >/proc/sys/kernel/perf_event_paranoid'

g++ -I../lib/entt-2.7.3/src/entt/ main.cpp -lpapi

./a.out

gnuplot -c barplot.plt "setup l1 and l2 d-cache miss" ../data/10000empty.dat > ../data/setupempty.png
gnuplot -c barplot.plt "setup with components l1 and l2 d-cache miss" ../data/withcomponents.dat > ../data/withcomponents.png
gnuplot -c barplot.plt "update components l1 and l2 d-cache miss" ../data/update.dat > ../data/update.png
gnuplot -c line.plt "cpu cycles vs creating entities" ../data/cyclesvscreation.dat > ../data/cyclesvscreation.png
gnuplot -c line.plt "cpu cycles vs reading from entities" ../data/cyclesvsreads.dat > ../data/cyclesvsreads.png
gnuplot -c line.plt "cpu cycles vs writing to entities" ../data/cyclesvswrites.dat > ../data/cyclesvswrites.png
