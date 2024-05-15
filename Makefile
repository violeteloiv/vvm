CFLAGS = -Wall -Wextra -Wswitch-enum -std=c11 -pedantic
LIBS   =

.PHONY = clean

.PHONY: all
all: vasm vmi

# $@: name of target, $^ is all the depedencies
vasm: vasm.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

vmi: vmi.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -rf vvm

.PHONY: examples
examples: ./examples/fib.vm ./examples/123.vm

./examples/fib.vm: ./examples/fib.vasm
	./vasm ./examples/fib.vasm ./examples/fib.vm

./examples/123.vm: ./examples/123.vasm
	./vasm ./examples/123.vasm ./examples/123.vm
