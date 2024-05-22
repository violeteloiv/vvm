# thank u sneha i love u <3

CFLAGS = -Wall -Wextra -Wswitch-enum -Wmissing-prototypes -std=c11 -pedantic
LIBS   =

EXAMPLES = ./examples/fib.vm ./examples/123i.vm ./examples/123f.vm ./examples/e.vm ./examples/pi.vm

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
	rm -rf ./examples/123i.vm
	rm -rf ./examples/123f.vm
	rm -rf ./examples/fib.vm
	rm -rf ./examples/e.vm
	rm -rf ./examples/pi.vm

.PHONY: examples
examples: $(EXAMPLES)

%.vm: %.vasm vasm
	./build/vasm $< $@