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

# Require user to run as root
if [ `whoami` != root ]; then
    echo "ERROR: Please run this script as root or using sudo (sudo ./run.sh)"
    exit
fi

# Require 1 input argument defining which userapp to run (1-4)
if [ "$#" -ne 1 ]; then
    echo "Illegal number of parameters"
    exit
fi

# cleanup
./reset.sh
find . -type f -exec touch {} +

# build and install device driver
echo "Building and installing device driver"
make
insmod driver.ko

# run user application for testing
case "$1" in
    "1")
        echo "Running userapp1"
        ./userapp1
        ;;

    "2")
        echo "Running userapp2"
        ./userapp2
        ;;
    
    "3")
        echo "Running userapp3"
        ./userapp3
        ;;
    
    "4")
        echo "Running userapp4"
        ./userapp4
        ;;

    *)
        echo "ERROR (run.sh): Acceptable input arguments: 1, 2, 3, 4"
esac

# Uninstall device driver
#echo "Uninstalling device driver"
#rmmod mycdrv.ko
