#!/bin/bash
# ----------------------------------------------------------
# FILE: kill.sh
# ----------------------------------------------------------
# SUMMARY: This script kills all processes with a handle to 
# reducer, mapper, and combiner executables.
# ----------------------------------------------------------
# USAGE: If the combiner program locks up, use this 
# script to allow files to be recompiled.
# ----------------------------------------------------------

lsof -t ./userapp1 | xargs kill &>/dev/null
lsof -t ./userapp2 | xargs kill &>/dev/null
lsof -t ./userapp3 | xargs kill &>/dev/null
lsof -t ./userapp4 | xargs kill &>/dev/null
echo "Handles to executables killed"
