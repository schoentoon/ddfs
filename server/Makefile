CFLAGS := $(CFLAGS) -Wall -O2 -mtune=native -g
INC    := -Iinclude $(INC)
LFLAGS := -levent
DEFINES:= $(DEFINES)
CC     := gcc
BINARY := isyf_server
DEPS   := ../build/main.o ../build/file_observer.o ../build/listener.o ../build/client.o ../build/file_callback.o

.PHONY: all clean dev

all: $(DEPS) link

dev: clean
	DEFINES="-DDEV" $(MAKE)

%.o: $(patsubst ../build/%o,src/%c,$@)
	$(CC) $(CFLAGS) $(DEFINES) $(INC) -c -o $@ $(patsubst ../build/%o,src/%c,$@)

link: $(DEPS)
	$(CC) $(CFLAGS) $(DEFINES) $(INC) -o ../bin/$(BINARY) $(DEPS) $(LFLAGS)

install:
	cp ../bin/$(BINARY) /usr/bin/$(BINARY)

clang:
	$(MAKE) dev CC=clang
