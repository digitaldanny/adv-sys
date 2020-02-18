# +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
# SUMMARY: run.sh 
# This script builds the combiner program, runs it, and outputs
# the resulting tuples to the test.txt file. The resulting 
# text file is compared with the provided output.txt file to
# check that the files are identical.
# +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
# NOTE:
# If the program is Ctrl-C'd, all relevant floating process IDs 
# can be killed by uncommenting the ./kill.sh command and
# rerunning the ./run script. 
# +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
# NOTE:
# This script touches all files so there is no clock skew issues
# with the makefile.
# +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+

#!/bin/bash
./reset.sh
find . -type f -exec touch {} +
make
./transfProg input.txt 1
