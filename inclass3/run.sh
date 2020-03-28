#!/bin/bash
make -f MakeHelloDriver
./HelloDriver
sudo dd if=/dev/tux0 bs=14 count=1