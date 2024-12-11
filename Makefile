CC = gcc
CFLAGS = -O3 -Wall -pedantic -std=c17 
HEADERS = ${wildcards *.h}

all: main

main: main.o 
	${CC} ${CFLAGS} -o $@ $^

%.o: %.c ${HEADERS}
	${CC} ${CFLAGS} -c $<

.PHONY: clean all

clean:
	rm -f *.o
	rm -f main
	