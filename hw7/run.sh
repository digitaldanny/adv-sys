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

# cleanup
./reset.sh
find . -type f -exec touch {} +

# build and install device driver
echo "Building and installing device driver"
make

DEV=/dev/usbkbd
if [ -c "$DEV" ]; then
    echo "$DEV already loaded."
else 
    echo "$DEV is not loaded. Registering now."
    insmod driver.ko
fi

# run user application for testing
echo "Running user application"
./userapp

# Uninstall device driver
#echo "Uninstalling device driver"
#rmmod mycdrv.ko