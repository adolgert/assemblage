CC = clang
CFLAGS = -Wall -Wextra -O2 -mavx2 -march=native -ftree-vectorize
LIBS = -lm

TARGET = dependent
SOURCES = dependent.c
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET) assembly llvm-ir

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.s
	$(CC) -c -o $@ $<

assembly: dependent.s

# this prints the number of consecutive calls in the assembly.
dependent.s: dependent.c
	$(CC) $(CFLAGS) -S -o $@ $<
	@python3 strip.py dependent.s | uniq -c | awk '$$1 >= 100 && /imull/' || true

llvm-ir: dependent.ll


# Default LLVM IR (matches object compilation)
dependent.ll: dependent.c
	$(CC) -S -emit-llvm $(CFLAGS) -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET) *.s *.ll

run:
	taskset -c 3 ./dependent

.PHONY: all assembly llvm-ir run clean run
