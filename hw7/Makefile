all: userapp clean

userapp: 
	@echo "========= building userapp ========="
	gcc -o userapp userapp.c -pthread

.PHONY : clean
clean:
	@echo "============= cleaning ============="
	$(shell find . -type f -iregex '.*\.\(tmp\|o\|mod\|cmd\|symvers\|order\|mod.*\)' -exec rm -f {} \;)