CFLAGS := $(CFLAGS) -Wall -O2 -mtune=native -g
INC    := -Iinclude $(INC)
LFLAGS := -levent
DEFINES:= $(DEFINES)
CC     := gcc

.PHONY: all clean dev server client install

all: build server client

server:
	$(MAKE) -C server

client:
	$(MAKE) -C client

dev: clean build
	DEFINES="-DDEV" $(MAKE)

build:
	-mkdir build bin

clean:
	rm -rfv build bin
	$(MAKE) -C server clean
	$(MAKE) -C client clean

clang:
	$(MAKE) dev CC=clang
