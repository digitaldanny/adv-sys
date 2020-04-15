Assignment 6 README

Instructions on Running

- To run the userapp for any deadlock described below, use the 'run' script associated with that case (run1 -> case1, run2 -> case2).
- Run the script in sudo mode.
- Allowed script arguments are 1, 2, 3, or 4.
- Example) sudo ./run.sh 1
- Example) sudo ./run.sh 2
- Script will automatically install compiled driver.
- User must uninstall driver using ```sudo rmmod driver.ko```.

Deadlock Cases

- 1.) When running in MODE 1, only one thread can open and access devices. If the same thread
tries to open the device twice, the device driver will get stuck on line 50 ```down_interruptible(&devc->sem2);```.
Because the current thread has already claimed devc->sem2 (a binary semaphore), the same thread will 
be unable to claim the semaphore again. This results in a deadlock in the main thread.

- 2.)

Data Race Cases

- 1.) 

Modifications to driver.c

- 1.) Added line 14 ```#include <linux/uaccess.h>``` to fix "implicit declaration" error.
- 2.) Changed line 27 to majornumber = 500 to avoid CHRDEV "a5" major requested (700) is greater than the maximum (511) error.