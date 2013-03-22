CFLAGS := $(CFLAGS) -Wall -O2 -mtune=native -g
INC    := -Iinclude $(INC)
LFLAGS := -levent
DEFINES:= $(DEFINES)
CC     := gcc

.PHONY: all clean dev server install

all: build server

server:
	$(MAKE) -C server

dev: clean build
	DEFINES="-DDEV" $(MAKE) -C server

build:
	-mkdir -p build bin

clean:
	rm -rfv build bin

clang:
	$(MAKE) dev CC=clang
