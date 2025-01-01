#Francesco Carollo SM3201419

CC = gcc
CFLAGS = -g -Wall -pedantic -std=c17 -lm -fopenmp
SRCDIR   = src
HEADDIR  = src
OBJDIR   = build

SOURCES  := $(wildcard $(SRCDIR)/*.c)
HEADERS := $(wildcard $(HEADDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

all: main

main: ${OBJECTS}
	${CC} ${CFLAGS} -o $@ $^

build/%.o: src/%.c ${HEADERS}
	${CC} ${CFLAGS} -c $< -o $@


clean:
	rm -f build/*.o
	rm -f main
	rm -f renders/*.ppm
	