#!/bin/bash

g++ -I../lib/entt-2.7.3/src/entt/ main.cpp -lpapi

./a.out

gnuplot -c barplot.plt "setup l1 and l2 d-cache miss" ../data/10000empty.dat > ../data/setupempty.png
gnuplot -c barplot.plt "setup with components l1 and l2 d-cache miss" ../data/withcomponents.dat > ../data/withcomponents.png
gnuplot -c barplot.plt "update components l1 and l2 d-cache miss" ../data/update.dat > ../data/update.png
gnuplot -c line.plt ../data/rampup.dat > ../data/rampup.png
