Assignment 6 README

This submission uses the original assignment6.c code with a few small modifications mentioned below
to avoid errors due to system version / configurations. The assignment6.c file was renamed to driver.c.

##############################################################
Modifications to driver.c
##############################################################

- 1.) Added line 14 ```#include <linux/uaccess.h>``` to fix "implicit declaration" error.
- 2.) Changed line 27 to majornumber = 500 to avoid CHRDEV "a5" major requested (700) is greater than the maximum (511) error.

##############################################################
Instructions on Running
##############################################################

- To run the userapp for any deadlock described below, use the 'run' script associated with that case (run1 -> case1, run2 -> case2).
- Run the script in sudo mode.
- Allowed script arguments are 1, 2, 3, or 4.
- Example to run demo for deadlock 1) sudo ./run.sh 1
- Example to run demo for deadlock 2) sudo ./run.sh 2
- Script will automatically install compiled driver.
- User must uninstall driver using ```sudo rmmod driver.ko```.

##############################################################
Deadlock Cases
##############################################################

- 1.) When running in MODE 1, only one thread can open and access devices. If the same thread
tries to open the device twice, the device driver will get stuck on line 50 ```down_interruptible(&devc->sem2);```.
Because the current thread has already claimed devc->sem2 (a binary semaphore), the same thread will 
be unable to claim the semaphore again. This results in a deadlock in the main thread.

Expected Terminal Output:
Open 1.
Open 1 complete.
Open 2.

- 2.) Create two threads. The first threads opens the device and sets it to MODE 2, which allows the 
second thread to open the device also. The second thread sets to MODE 1, which causes a deadlock for
thread 1 operation.

Expected Terminal Output:
T1: Opened device.
T1: Set device to MODE 2.
T2: Opened device.
T2: Set device to MODE 1.

- 3.) Create two threads. The first thread opens the device. The second thread opens the device but is blocked
indefinitely while waiting for sem2. The first thread tries to switch to mode 2 by using ioctl(MODE_2) but is
block indefinitely while waiting on queue 1. 

Expected Terminal Output:
T1: Opened device.
T2: Opening device.
T1: Switching to MODE 2.

- 4.) Create two threads. The first thread opens the device and switches into mode 2 using ioctl(MODE_2). The 
second thread opens the device successfully. Both threads try to switch to mode 1 using ioctl(MODE_1) but block
indefinitely while waiting on queue 2.

Expected Terminal Output:
T1: Opened device.
T1: Switched to MODE 2.
T2: Opened device.
T2: Switching to MODE 1.
T1: Switching to MODE 1.

##############################################################
Data Race Cases
##############################################################

- 1.) Thread 1 opens device with global file descriptor (fd) in MODE 1. Thread 1 starts performing reads / writes. 
Thread 2 does not have to open the device because it is already opened with the global fd, so thread 2 starts performing 
reads / writes without opening a new file descriptor. This is a data race for the device buffer. 

Critical region start   : Thread 1 open
Critical region end     : Thread 1 release
Data Accessed           : devc->ramdisk
Locks Held at Beginning : sem2
Possiblity of Data Race : There is possiblity of data race because the sem2 lock is being bypassed by not using the open
	and release functions inside of thread 2. Inside of the read/write functions for MODE 1, sem1 is unlocked before
	copying data to/from the device which means the ramdisk is exposed to multiple threads.

- 2.) This case is similar to the first data race case, except with multi-process usage. The parent process opens
the device with file descriptor (fd). Then the process forks and the child process can access the device without
opening the device using fd. Again, this is a data race for the device buffer. The critical region is between
parent process open and parent process close.

Critical region start   : Parent process open
Critical region end     : Parent process release
Data Accessed           : devc->ramdisk
Locks Held at Beginning : sem2
Possiblity of Data Race : There is possiblity of data race because the sem2 lock is being bypassed by not using the open
	and release functions inside the child process.

- 3.) Two threads open the device in MODE 2. They both call ioctrl to set the device to MODE 1 at the same time.

Critical region start   : After line 168 "down_interrruptible(&devc->sem1);" inside e2_ioctrl.
Critical region end     : After line 184 "up(&devc->sem1);" inside e2_ioctrl.
Data Accessed           : devc->count1, devc->count2
Locks Held at Beginning : sem1, sem2
Possiblity of Data Race : There is no possibility of a data race because sem1 protects count1 and count2. Locking sem2
	will cause a deadlock; however, it does not cause any data races either.

- 4.) Main thread opens a global file descriptor (fd) in MODE 1. Threads 1 and 2 bypass the open and release functions
and both call ioctrl to set the device into MODE 2 at the same time.

Critical region start   : After line 148 "down_interrruptible(&devc->sem1);" inside e2_ioctrl.
Critical region end     : After line 164 "up(&devc->sem1);" inside e2_ioctrl.
Data Accessed           : devc->count1, devc->count2
Locks Held at Beginning : sem1
Possiblity of Data Race : There is no possiblility of a data race here either even though both threads bypass the open
	and release functions. The sem1 lock blocks concurrent access to count1/count2 so those are protected.