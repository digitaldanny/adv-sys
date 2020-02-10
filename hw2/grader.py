'''
SUMMARY: grader
This script runs the combiner program from ASP HW2 and checks for deadlocks
and correct outputs.

TESTS RUN:

Deadlocks - This script will run the 'combiner.c' program "RUN_COUNT" number of times.
    The script will automatically terminate the program if it hangs. A program will
    be determined to have no deadlock issues if it can run RUN_COUNT number of times
    without hanging.

Correct Output - This script will generate a new input file with randomly ordered tuples.
    The script will run the 'combiner.c' program and check if the generated output file
    contains the same tuples as the example output text file from HW1.
'''

# +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
#                              CONFIGURATIONS
# +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
# RUN_COUNT - number of times the program will be run to test for deadlocks.
# TIMEOUT_PERIOD - number of seconds before iteration of combiner is killed.
# COMBINER_DIR - directory of the combiner executable.
# EXAMPLE_INPUT_DIR - directory of the HW1 example input file.
# EXAMPLE_OUTPUT_DIR - directory of the HW1 example output file.
# TEST_INPUT_DIR - directory of generated input file for testing.
# TEST_OUTPUT_DIR - directory of generated output file after testing TEST_INPUT_DIR.
# +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
RUN_COUNT               = 10
TIMEOUT_PERIOD          = 1
COMBINER_DIR            = './combiner'
EXAMPLE_INPUT_DIR       = './input.txt'
EXAMPLE_OUTPUT_DIR      = './output.txt'
TEST_INPUT_DIR          = './test_input.txt'
TEST_OUTPUT_DIR         = './test_output.txt'
# +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+

import sys, os, signal
import re
import random
import subprocess
import time

'''
SUMMARY: main
Main program runs combiner program RUN_COUNT number of times and checks outputs
with given output file to test for deadlocks/correct output.
'''
def main():
    global RUN_COUNT
    success = 0
    print("BEGIN TEST!")
    print("+====="*10+"+")

    # run the combiner program RUN_COUNT number of times and 
    # record whether the run was successful or not.
    generateRandomInputFile()
    for i in range(RUN_COUNT):
        if(runCombinerProgram(i) != 1): 
            continue
        if(compareOutputFiles()): 
            success = success+1
    
    # check if every run through the program was successful.
    print("+====="*10+"+")
    if (success == RUN_COUNT):
        print("SUCCESS: All tests passed.")
    else:
        print("FAILURE: Passed "+str(success)+"/"+str(RUN_COUNT)+" runs.")
    print("+====="*10+"+")
    return

'''
SUMMARY: test_current_files
This program tests that the current EXAMPLE_OUTPUT_DIR file and TEST_OUTPUT_DIR
file contain the same tuples.
'''
def test_current_files():
    compareOutputFiles()
    return

'''
SUMMARY: generateRandomInputFile
'''
def generateRandomInputFile():
    global EXAMPLE_INPUT_DIR
    rfile = []
    wfile = []
    data = []

    # Read in all tuples as a string from the file.
    print("Generating new input file from " + EXAMPLE_INPUT_DIR)
    try:
        rfile = open(EXAMPLE_INPUT_DIR, 'r') 
        dataString = rfile.readline()
        rfile.close()
    except:
        print("ERROR: Example input directory ("+EXAMPLE_INPUT_DIR+") does not exist.")
        sys.exit()

    # Extract the tuples from the string.
    data = re.findall(r'\(.*?\)',dataString)

    # Create a text file and randomly write each tuple to the file
    print("Generating new test input file as " + TEST_INPUT_DIR)
    try:
        try:
            print("Removing test input file.")
            os.remove(TEST_INPUT_DIR)
        except:
            print("Test input file does not exist. Creating new file.")

        wfile = open(TEST_INPUT_DIR, "w")
        print("Writing "+str(len(data))+" tuples to test file.")

        for i in range(len(data)):
            Tuple = random.choice(data) # randomly choose a tuple to write to file
            wfile.write(Tuple)
            if (len(data) > 1): wfile.write(',')
            data.remove(Tuple) # remove tuple from the list so it doesn't repeat
        wfile.close()

    except:
        print("ERROR: Could not generate test input file ("+TEST_INPUT_DIR+")")
        sys.exit()

    return

'''
SUMMARY: runCombinerProgram
This function runs two processes. The child process will run the combiner
program while the parent process waits for the child to terminate. After
a short period of time, the parent process will kill the child if it is
hanging (deadlock).
'''
def runCombinerProgram(runNum):
    print("+-----"*10 +"+")
    print("\t\t\tRunning test #" + str(runNum))
    print("+-----"*10 +"+")

    pid = os.fork()
    if pid == 0:
        # CHILD PROCESS
        subprocess.call("make")
        fin = open(TEST_INPUT_DIR, "r")
        fout = open(TEST_OUTPUT_DIR, "w")
        subprocess.call(COMBINER_DIR, stdin=fin, stdout=fout)
        fin.close()
        fout.close()
        os._exit(0)

    else:
        # PARENT PROCESS
        #child_id, status = os.waitpid(pid, 0)
        global TIMEOUT_PERIOD
        timer = 0

        # wait for the combiner to complete OR timeout..
        time.sleep(TIMEOUT_PERIOD)
        try:
            os.kill(pid, signal.SIGSTOP) 
            info = os.waitpid(pid, 0) 
            print("TIMEOUT: Combiner process hung and was terminated by parent.")
            return 0

        except:
            print("Combiner process executed successfully.")

    print("\n")
    return 1

'''
SUMMARY: compareOutputFiles
This function compares the tuples in the current test file and the 
provided example output file.
'''
def compareOutputFiles():
    global TEST_OUTPUT_DIR
    global EXAMPLE_OUTPUT_DIR
    errorCount = 0

    fTest = open(TEST_OUTPUT_DIR, 'r')
    fExample = open(EXAMPLE_OUTPUT_DIR, 'r')

    # instantiate map that will contain the read tuples.
    tuples = dict()

    # first read in the example output tuples.
    for line in fExample:
        tuples[line] = 1
    
    # read in the test output tuples.
    for line in fTest:
        # Check if the keys exist inside of the dictionary already.
        # If yes, test output file contains a correct tuple.
        # If not, the combiner program is missing a tuple.
        if line in tuples:
            tuples[line] = 2
        else:
            errorCount = errorCount + 1
            print("Incorrect output in test file: " + line)

    # check if any tuples were in the generated output file but were 
    # not found in the test output file.
    for key in tuples:
        if tuples[key] == 1:
            print("Missing tuple from the test file: " + key)
            errorCount = errorCount + 1

    # print out number of differences found between expected output
    # and combiner's output.
    print("Number of errors found: " + str(errorCount))

    fTest.close()
    fExample.close()
    return 0

if __name__ == '__main__':
    #main()
    test_current_files()