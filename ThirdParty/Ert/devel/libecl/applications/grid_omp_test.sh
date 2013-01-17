#!/bin/csh
time ./grid_dump.x $1 /tmp/grid_n


setenv OMP_NUM_THREADS 32
time ./grid_dump.x $1 /tmp/grid_32

setenv OMP_NUM_THREADS 1
time ./grid_dump.x $1 /tmp/grid_1

unsetenv OMP_NUM_THREADS

diff /tmp/grid_1 /tmp/grid_n
diff /tmp/grid_1 /tmp/grid_32
