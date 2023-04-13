APP=gen_expr

$(APP): gen_expr.c
	gcc -O2 -o $@ $<

.PHONY: clean
clean:
	-rm $(APP) .code.c .expr