#!/bin/bash
find . -name "*.o" -type f -delete
find . -name "*.tmp" -type f -delete
find . -name "*.cmd" -type f -delete
find . -name "*.mod" -type f -delete
find . -name "*.symvers" -type f -delete
find . -name "*.order" -type f -delete
find . -name "*.mod.*" -type f -delete
find . -name "*.ko" -type f -delete
find . -name "userapp1" -type f -delete
find . -name "userapp2" -type f -delete
find . -name "userapp3" -type f -delete
find . -name "userapp4" -type f -delete
rm userapp
