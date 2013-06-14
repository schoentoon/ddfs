export CC     := gcc
export CFLAGS := -Wall -O2 -mtune=native -g
export LFLAGS := -levent
VERSION:= $(shell git describe)
ifdef NO_OPENSSL
  export DEFINES := ${DEFINES} -DNO_OPENSSL -DVERSION=\"$(VERSION)\"
else
  export LFLAGS := -levent -levent_openssl -lssl -lcrypto
  export DEFINES:= ${DEFINES} -DVERSION=\"$(VERSION)\"
endif

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
