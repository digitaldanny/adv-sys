Assignment 6 README

Instructions on Running

- To run the userapp for any deadlock described below, use the 'run' script associated with that case (run1 -> case1, run2 -> case2).
- Run the script in sudo mode.
- Allowed script arguments are 1, 2, 3, or 4.
- Example) sudo ./run.sh 1
- Example) sudo ./run.sh 2

Deadlock Cases

- 1.)

Data Race Cases

- 1.)

Modifications to driver.c

- 1.) Added line 14 ```#include <linux/uaccess.h>``` to fix "implicit declaration" error.