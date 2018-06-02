CFLAGS=-g $(shell sdl2-config --cflags)
LDLIBS=$(shell sdl2-config --libs)

all: listdevs inittest restoretty inittest-dyn

inittest-dyn: LDLIBS=-ldl
inittest-dyn: inittest-dyn.o

inittest-dyn.o: inittest.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -DDYNAMIC_LOAD $< -o $@
