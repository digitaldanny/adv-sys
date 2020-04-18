## if KERNELRELEASE is not defined, we've been called directly from the command line.
## Invoke the kernel build system.
ifeq (${KERNELRELEASE},)
	KERNEL_SOURCE := /usr/src/linux-headers-$(shell uname -r)
	PWD := $(shell pwd)

all: driver userapp1 userapp2 userapp3 userapp4 clean

driver:
	@echo "========= building driver =========="
	${MAKE} -C ${KERNEL_SOURCE} M=${PWD} modules

userapp1: 
	@echo "========= building userapp1 ========="
	gcc -std=c99 -o userapp1 userapp1.c

userapp2: 
	@echo "========= building userapp2 ========="
	gcc -std=c99 -o userapp2 userapp2.c -pthread

userapp3: 
	@echo "========= building userapp3 ========="
	gcc -std=c99 -o userapp3 userapp3.c -pthread

userapp4: 
	@echo "========= building userapp4 ========="
	gcc -std=c99 -o userapp4 userapp4.c -pthread

.PHONY : clean
clean:
	@echo "============= cleaning ============="
	$(shell find . -type f -iregex '.*\.\(tmp\|o\|mod\|cmd\|symvers\|order\|mod.*\)' -exec rm -f {} \;)

# Otherwise KERNELRELEASE is defined; we've been invoked from the
# kernel build system and can use its language.
else
	obj-m += driver.o
	#obj-m += mycdrv.o
	#mycdrv-y := char_driver.o
endif