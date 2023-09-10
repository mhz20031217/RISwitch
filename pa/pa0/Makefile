CC = gcc
LD = gcc
DBG = gdb
CFLAGS = -g -Wall -Werror
LDFLAGS = 


BIN = hello
OBJS = hello.o

SRCS = $(patsubst %.o, %.c, $(OBJS))

.PHONY: build run debug clean

default: build

build: $(BIN)

$(BIN): $(OBJS)
	$(LD) -o $@ $(OBJS)

$(OBJS): $(SRCS)
	$(CC) -c $(CFLAGS) $<

run: build
	$(abspath $(BIN))

debug: build
	$(DBG) $(BIN)

clean:
	-rm -f $(BIN) $(OBJS)
