CC=gcc
CFLAGS=-g -Wall -Wextra -std=gnu99 $(shell pkg-config --cflags cairo)
LDLIBS=-lm $(shell pkg-config --libs cairo)

all: scan

scan: wcairo.o wosm.o cairoexport.o tracer.o memimg.o layer.o smlog.o

clean:
	rm -f *.o wolken scan

.PHONY: clean

