CFLAGS := $(CFLAGS) -Wall -O2 -mtune=native -g
INC    := -Iinclude $(INC)
LFLAGS := -levent
DEFINES:= $(DEFINES)
CC     := gcc
BINARY := isyf
DEPS   := build/main.o build/file_observer.o

.PHONY: all clean dev

all: build $(DEPS) link

dev: clean
	DEFINES="-DDEV" $(MAKE)

build:
	-mkdir -p build bin

%.o: $(patsubst build/%o,src/%c,$@)
	$(CC) $(CFLAGS) $(DEFINES) $(INC) -c -o $@ $(patsubst build/%o,src/%c,$@)

link: $(DEPS)
	$(CC) $(CFLAGS) $(DEFINES) $(INC) -o bin/$(BINARY) $(DEPS) $(LFLAGS)

clean:
	rm -rfv build bin

install:
	cp bin/$(BINARY) /usr/bin/$(BINARY)

clang:
	$(MAKE) dev CC=clang
