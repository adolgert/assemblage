CC = clang
CFLAGS = -Wall -Wextra -O0 -g

TARGET = gemm_naive
SOURCES = main.c gemm_naive.c
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET) assembly llvm-ir

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.s
	$(CC) -c -o $@ $<

assembly: gemm_naive.s gemm_arm.s gemm_risc.s

gemm_naive.s: gemm_naive.c
	$(CC) $(CFLAGS) -S -o $@ $<

gemm_arm.s: gemm_naive.c
	$(CC) --target=aarch64-linux-gnu --sysroot=/usr/aarch64-linux-gnu -O0 -g -S -o $@ $<

gemm_risc.s: gemm_naive.c
	$(CC) --target=riscv64-linux-gnu --sysroot=/usr/riscv64-linux-gnu -O0 -g -S -o $@ $<

llvm-ir: gemm_naive.ll gemm_naive-opt.ll gemm_naive-unopt.ll

# Unoptimized LLVM IR
gemm_naive-unopt.ll: gemm_naive.c
	$(CC) -S -emit-llvm -O0 -o $@ $<

# Optimized LLVM IR (same as CFLAGS)
gemm_naive-opt.ll: gemm_naive.c
	$(CC) -S -emit-llvm -O3 -o $@ $<

# Default LLVM IR (matches object compilation)
gemm_naive.ll: gemm_naive.c
	$(CC) -S -emit-llvm $(CFLAGS) -o $@ $<

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET) *.s *.ll

.PHONY: all assembly llvm-ir run clean
