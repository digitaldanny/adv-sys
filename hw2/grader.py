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
# COMBINER_DIR - directory of the combiner executable.
# EXAMPLE_INPUT_DIR - directory of the HW1 example input file.
# EXAMPLE_OUTPUT_DIR - directory of the HW1 example output file.
# TEST_INPUT_DIR - directory of generated input file for testing.
# TEST_OUTPUT_DIR - directory of generated output file after testing TEST_INPUT_DIR.
# +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
RUN_COUNT               = 10
COMBINER_DIR            = './combiner'
EXAMPLE_INPUT_DIR       = './input.txt'
EXAMPLE_OUTPUT_DIR      = './output.txt'
TEST_INPUT_DIR          = './test_input.txt'
TEST_OUTPUT_DIR         = './test_output.txt'
# +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+

import sys, os
import re
import random

def main():
    generateRandomInputFile()
    runCombinerProgram()
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
'''
def runCombinerProgram():
    return

'''
SUMMARY: compareOutputFiles
'''
def compareOutputFiles():
    return

if __name__ == '__main__':
    main()