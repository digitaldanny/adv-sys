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

#!/bin/bash
#./kill.sh
make
./combiner < input.txt | tee test.txt
diff -q -y -s output.txt test.txt
