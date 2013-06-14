CC     := ${CC}
CFLAGS := ${CFLAGS}
INC    := -Iinclude -I../global ${INC}
LFLAGS := ${LFLAGS}
BINARY := ddfs_server
DEPS   := build/main.o build/file_observer.o build/listener.o build/client.o build/file_callback.o build/log.o

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

clean:
	rm -rfv build

install:
	cp -f ../bin/$(BINARY) /usr/bin/$(BINARY)

clang:
	$(MAKE) dev CC=clang
