#!/bin/bash
find . -name "*.o" -type f -delete
find . -name "reducer" -type f -delete
find . -name "mapper" -type f -delete
find . -name "combiner" -type f -delete
find . -name "test.txt" -type f -delete
find . -name "test_input.txt" -type f -delete
find . -name "test_output.txt" -type f -delete