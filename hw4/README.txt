To build and run the combiner program, go into the submission folder in a terminal and 
type in "./run.sh"

RUN.SH NOTES:

The 'run' script will use the make script to build the combiner program and execute
the combiner program with the following parameters

./combiner bufferSize numberOfProcesses < input.txt > test_output.txt

where the script uses bufferSize = 10 and numberOfProcesses = 7.
