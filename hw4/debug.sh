#!/bin/bash
fuser -k combiner
make
#tmux attach -t asp
gdb --command=gef_cmd.txt combiner
