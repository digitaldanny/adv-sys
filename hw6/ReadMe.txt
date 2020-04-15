Assignment 6 README

This submission uses the original assignment6.c code with a few small modifications mentioned below
to avoid errors due to system version / configurations. The assignment6.c file was renamed to driver.c.

Modifications to driver.c

- 1.) Added line 14 ```#include <linux/uaccess.h>``` to fix "implicit declaration" error.
- 2.) Changed line 27 to majornumber = 500 to avoid CHRDEV "a5" major requested (700) is greater than the maximum (511) error.

Instructions on Running

- To run the userapp for any deadlock described below, use the 'run' script associated with that case (run1 -> case1, run2 -> case2).
- Run the script in sudo mode.
- Allowed script arguments are 1, 2, 3, or 4.
- Example to run demo for deadlock 1) sudo ./run.sh 1
- Example to run demo for deadlock 2) sudo ./run.sh 2
- Script will automatically install compiled driver.
- User must uninstall driver using ```sudo rmmod driver.ko```.

Deadlock Cases

- 1.) When running in MODE 1, only one thread can open and access devices. If the same thread
tries to open the device twice, the device driver will get stuck on line 50 ```down_interruptible(&devc->sem2);```.
Because the current thread has already claimed devc->sem2 (a binary semaphore), the same thread will 
be unable to claim the semaphore again. This results in a deadlock in the main thread. The demo (userapp1)
will infinite loop after outputting "Open 2."

- 2.) Create two threads. The first threads opens the device and sets it to MODE 2, which allows the 
second thread to open the device also. 

Data Race Cases

- 1.) 