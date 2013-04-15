CFLAGS := $(CFLAGS) -Wall -O2 -mtune=native -g
INC    := -Iinclude -I../global $(INC)
LFLAGS := -levent -lpthread
VERSION:= $(shell git describe)
DEFINES:= $(DEFINES) -DVERSION=\"$(VERSION)\"
CC     := gcc
BINARY := ddfs
DEPS   := build/main.o build/client.o build/log.o build/hook.o

.PHONY: all clean dev

all: build $(DEPS) link

build:
	-mkdir build

dev: clean
	DEFINES="-DDEV" $(MAKE)

build/log.o: ../global/log.h ../global/log.c
	$(CC) $(CFLAGS) $(DEFINES) $(INC) -c -o build/log.o ../global/log.c

%.o: $(patsubst build/%o,src/%c,$@)
	$(CC) $(CFLAGS) $(DEFINES) $(INC) -c -o $@ $(patsubst build/%o,src/%c,$@)

link: $(DEPS)
	$(CC) $(CFLAGS) $(DEFINES) $(INC) -o ../bin/$(BINARY) $(DEPS) $(LFLAGS)

windows:
	$(MAKE) dev CC=i686-w64-mingw32-gcc LFLAGS="-lws2_32 -levent"

clean:
	rm -rfv build

install:
	cp -f ../bin/$(BINARY) /usr/bin/$(BINARY)

clang:
	$(MAKE) dev CC=clang
