# +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
# SUMMARY: run.sh 
# This script runs the combiner program 10 times and returns 
# the number of times the process resulted in a deadlock.
# +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+

DEADLOCK_THRESH = 3     # seconds before a process is considered deadlocked
DEADLOCK_COUNT  = 0     # keeps track of deadlocks

#!/bin/bash
#./kill.sh
fuser -k combiner
./reset.sh
find . -type f -exec touch {} +
make

for i in {0..10..1}
do
    # generate a new input file for testing
    python grader.py gen

    # run the combiner program.. if timer exceeds DEADLOCK_THRESH, increment fail count.
    ./combiner 10 7 < input.txt | tee test_output.txt &
    pid = $! # save the process id of the combiner program

    echo $i
    wait $pid

    # grade the results of the generated output file.
    python grader.py grade
    #kill $pid
done
