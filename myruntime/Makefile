CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra
LDFLAGS ?= -lcap

SRC := src/main.c
OUT := build/myruntime

.PHONY: all run clean

all: $(OUT)

$(OUT): $(SRC)
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

run: $(OUT)
	sudo $(OUT)

clean:
	@rm -f $(OUT)
