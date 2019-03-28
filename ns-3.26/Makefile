# Makefile wrapper for waf

ADDITIONAL_OBJCFLAGS = -Wno-error

all:
	./waf

# free free to change this part to suit your requirements
configure:
	./waf configure --enable-examples --enable-tests

build:
	./waf build

install:
	./waf install

clean:
	./waf clean

distclean:
	./waf distclean


