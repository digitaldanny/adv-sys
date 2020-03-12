#!/bin/bash
fuser -k combiner
./reset.sh
make
#tmux attach -t asp
gdb --command=gef_cmd.txt combiner
