CFLAGS := $(CFLAGS) -Wall -O2 -mtune=native -g
INC    := -Iinclude $(INC)
LFLAGS := -levent
VERSION:= $(shell git describe)
DEFINES:= $(DEFINES) -DVERSION=\"$(VERSION)\"
CC     := gcc
BINARY := ddfs_server
DEPS   := build/main.o build/file_observer.o build/listener.o build/client.o build/file_callback.o

.PHONY: all clean dev

all: build $(DEPS) link

build:
	-mkdir build

dev: clean
	DEFINES="-DDEV" $(MAKE)

%.o: $(patsubst build/%o,src/%c,$@)
	$(CC) $(CFLAGS) $(DEFINES) $(INC) -c -o $@ $(patsubst build/%o,src/%c,$@)

link: $(DEPS)
	$(CC) $(CFLAGS) $(DEFINES) $(INC) -o ../bin/$(BINARY) $(DEPS) $(LFLAGS)

clean:
	rm -rfv build

install:
	cp ../bin/$(BINARY) /usr/bin/$(BINARY)

clang:
	$(MAKE) dev CC=clang
