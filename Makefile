-include nemu/Makefile.git

default:
	@echo "Please run 'make' under any subprojects to compile."
 
clean:
	-$(MAKE) -C nemu clean
	-$(MAKE) -C nexus-am clean
	-$(MAKE) -C nanos-lite clean
	-$(MAKE) -C navy-apps clean

submit: clean
	git gc
	cd .. && tar cj $(shell basename `pwd`) > $(STU_ID).tar.bz2

.PHONY: default clean submit

NO_COLOR=\033[0m
OK_COLOR=\033[32;01m
ERROR_COLOR=\033[31;01m


CURRENET_LINE_COUNT = $(shell find ./nemu/  -name "*.[ch]"  | xargs cat | grep -Ev "^$$" | wc -l)
ADD_LINE_COUNT = $(shell expr $(CURRENET_LINE_COUNT) - 2817)
count:
	@echo "$(NO_COLOR)$(CURRENET_LINE_COUNT) $(OK_COLOR) lines in nemu now !"
	@echo "$(NO_COLOR)$(ADD_LINE_COUNT) $(OK_COLOR) lines added now !"
	
