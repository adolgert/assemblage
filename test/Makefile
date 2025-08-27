CC = gcc
CFLAGS = -Wall -Wextra -O2
LIBS = -lopenblas

TARGET = against_blas
SOURCE = against_blas.c

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean