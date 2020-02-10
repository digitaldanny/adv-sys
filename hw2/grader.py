'''
SUMMARY: grader
This script checks if the combiner program has produced the correct tuples 
according to the example output file provided in HW2. Additionally, a new 
randomized test input file will be generated every time this script is 
run to help check that the combiner program works for all input files.
'''

# +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
#                              CONFIGURATIONS
# +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
# RUN_COUNT - number of times the program will be run to test for deadlocks.
# TIMEOUT_PERIOD - number of seconds before iteration of combiner is killed.
# EXAMPLE_INPUT_DIR - directory of the HW1 example input file.
# EXAMPLE_OUTPUT_DIR - directory of the HW1 example output file.
# TEST_INPUT_DIR - directory of generated input file for testing.
# TEST_OUTPUT_DIR - directory of generated output file after testing TEST_INPUT_DIR.
# +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
RUN_COUNT               = 10
TIMEOUT_PERIOD          = 1
EXAMPLE_INPUT_DIR       = './input.txt'
EXAMPLE_OUTPUT_DIR      = './output.txt'
TEST_INPUT_DIR          = './test_input.txt'
TEST_OUTPUT_DIR         = './test_output.txt'
# +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+

import re, random

'''
SUMMARY: test_current_files
This program tests that the current EXAMPLE_OUTPUT_DIR file and TEST_OUTPUT_DIR
file contain the same tuples. Additionally, it generates a new randomized input
file for the next time the test is run.
'''
def main():
    try:
        compareOutputFiles()
        generateRandomInputFile()
    except:
        print("Error running grader.py")
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
    main()