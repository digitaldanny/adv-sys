# +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
# SUMMARY: run.sh 
# This script builds the combiner program, runs it, and outputs
# the resulting tuples to the test_ouput.txt file.
# +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
# NOTE:
# This script touches all files so there is no clock skew issues
# with the makefile.
# +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+

#!/bin/bash
#./kill.sh
fuser -k combiner
./reset.sh
find . -type f -exec touch {} +
make
./combiner 10 10 < input.txt | tee test_output.txt
