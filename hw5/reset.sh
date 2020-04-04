#!/bin/bash
find . -name "*.o" -type f -delete
find . -name "*.tmp" -type f -delete
find . -name "*.cmd" -type f -delete
find . -name "*.ko" -type f -delete
find . -name "*.mod" -type f -delete
find . -name "*.symvers" -type f -delete
find . -name "*.order" -type f -delete
find . -name "*.mod.*" -type f -delete
rm userapp
