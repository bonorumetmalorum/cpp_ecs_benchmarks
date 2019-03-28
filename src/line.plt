set terminal png
set title "cpu cycles with increasing writes"
set xlabel "number of entities"
set ylabel "number of cpu cycles"
plot ARG1 using 1:2 title 'cpu cycles' with lines##, ARG1 using 1:3 title 'L1 cache miss', ARG1 using 1:4 title 'L2 cache miss' with lines
