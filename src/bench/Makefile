CC = clang
CFLAGS = -Wall -Wextra -O2 -g

TARGET = dependent
SOURCES = dependent.c
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET) assembly llvm-ir

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.s
	$(CC) -c -o $@ $<

assembly: dependent.s

dependent.s: dependent.c
	$(CC) $(CFLAGS) -S -o $@ $<

llvm-ir: dependent.ll


# Default LLVM IR (matches object compilation)
dependent.ll: dependent.c
	$(CC) -S -emit-llvm $(CFLAGS) -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET) *.s *.ll

.PHONY: all assembly llvm-ir run clean
