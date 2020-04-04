## if KERNELRELEASE is not defined, we've been called directly from the command line.
## Invoke the kernel build system.
ifeq (${KERNELRELEASE},)
	KERNEL_SOURCE := /usr/src/linux-headers-$(shell uname -r)
	PWD := $(shell pwd)

all: driver app clean

driver:
	@echo "========= building driver =========="
	${MAKE} -C ${KERNEL_SOURCE} M=${PWD} modules

app: 
	@echo "========= building userapp ========="
	gcc -std=c99 -o userapp userapp.c

.PHONY : clean
clean:
	@echo "============= cleaning ============="
	$(shell find . -type f -iregex '.*\.\(tmp\|o\|mod\|cmd\|symvers\|order\|mod.*\)' -exec rm -f {} \;)

# Otherwise KERNELRELEASE is defined; we've been invoked from the
# kernel build system and can use its language.
else
	obj-m += char_driver.o
	#obj-m += mycdrv.o
	#mycdrv-y := char_driver.o
endif