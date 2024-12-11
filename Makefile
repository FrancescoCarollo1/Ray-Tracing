#Francesco Carollo SM3201419

CC = gcc
CFLAGS = -O3 -Wall -pedantic -std=c17 
HEADERS = ${wildcards *.h}

all: main

main: main.o ppm.o scene.o
	${CC} ${CFLAGS} -o $@ $^

%.o: %.c ${HEADERS}
	${CC} ${CFLAGS} -c $<

.PHONY: clean all

clean:
	rm -f *.o
	rm -f main
	