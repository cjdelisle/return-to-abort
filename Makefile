all: demo

## Without -O0 the protected functions might be merged which breaks the poc
CFLAGS += -fno-stack-protector -O0 -D_FORTIFY_SOURCE=0
CC ?= gcc

clean:
	rm -f demo *.s *.i

demo: demo.safe.s
	$(CC) -o $@ $^

%.s: %.safereturn.i
	$(CC) $(CFLAGS) -S -o $@ $^

%.safereturn.i: %.i safereturn.py
	python safereturn.py $< > $@
	diff $< $@ || true

%.i: %.c
	$(CC) $(CFLAGS) -E -o $@ $^


%.safe.s: %.s safecall.py
	python safecall.py $< > $@
	diff $< $@ || true

.PRECIOUS: %.s %.safe.s
.PHONY: clean all