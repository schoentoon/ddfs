CFLAGS := $(CFLAGS) -Wall -O2 -mtune=native -g
INC    := -Iinclude $(INC)
LFLAGS := -levent
DEFINES:= ${DEFINES}
ifdef NO_OPENSSL
  DEFINES := ${DEFINES} -DNO_OPENSSL
endif
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

install:
	$(MAKE) -C server install
	$(MAKE) -C client install

clang:
	$(MAKE) dev CC=clang
