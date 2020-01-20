To run the combiner executable, type ./run.sh into the terminal. The script will perform the
following functions..

1.) make

	Compiles all the source files to produce 3 executables.

2.) ./combiner < input.txt | tee test.txt

	Runs the combiner program using the input.txt file provided by Professor Yavuz on Canvas
	and stores the results in test.txt AND the terminal.

3.) diff -q -y -s output.txt test.txt

	Compares the provided output.txt file with the test.txt file and outputs any differences
	between the files side-by-side on the terminal. The resulting output should be 
	"Files output.txt and test.txt are identical."
