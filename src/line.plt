set terminal png
set title "ramp up l1 and l2 cache miss"
plot ARG1 using 1:2 title 'L1 cache miss' with lines, ARG1 using 1:3 title 'L2 cache miss' with lines