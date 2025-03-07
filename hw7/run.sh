#!/bin/bash
# +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
# SUMMARY: run.sh 
# This script builds and installs character device driver, 
# runs the test user application, and uninstalls the driver.
# +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
# NOTE:
# This script touches all files so there is no clock skew issues
# with the makefile.
# +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+

# cleanup
fuser -k userapp
./reset.sh
find . -type f -exec touch {} +

# build and install device driver
echo "Building simulation"
make

# run user application for testing
echo "--------------------------------------------"
echo "Running first testcase"
echo "--------------------------------------------"
./userapp < input.txt

echo "--------------------------------------------"
echo "Running second testcase"
echo "--------------------------------------------"
./userapp < input2.txt
