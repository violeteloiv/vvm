# thank u sneha i love u <3

CFLAGS = -Wall -Wextra -Wswitch-enum -Wmissing-prototypes -std=c11 -pedantic
LIBS   =

.PHONY = clean

.PHONY: all
all: vasm vme devasm

# $@: name of target, $^ is all the depedencies
vasm: ./src/vasm.c ./src/vvm.h
	$(CC) $(CFLAGS) -o ./build/$@ $^ $(LIBS)

vme: ./src/vme.c ./src/vvm.h
	$(CC) $(CFLAGS) -o ./build/$@ $^ $(LIBS)

devasm: ./src/devasm.c ./src/vvm.h
	$(CC) $(CFLAGS) -o ./build/$@ $^ $(LIBS)

clean:
	rm -rf ./build/vasm
	rm -rf ./build/vme
	rm -rf ./build/devasm
	rm -rf ./examples/123.vm
	rm -rf ./examples/fib.vm

.PHONY: examples
examples: ./examples/fib.vm ./examples/123.vm

./examples/fib.vm: ./examples/fib.vasm
	./build/vasm ./examples/fib.vasm ./examples/fib.vm

./examples/123.vm: ./examples/123.vasm
	./build/vasm ./examples/123.vasm ./examples/123.vm
