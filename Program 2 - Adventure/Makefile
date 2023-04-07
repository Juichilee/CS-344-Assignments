CC=gcc
CFLAGS=-I

all: leejuic.buildrooms leejuic.adventure

leejuic.buildrooms.o: leejuic.buildrooms.c
	$(CC) -Wall -ansi -c leejuic.buildrooms.c

leejuic.adventure.o: leejuic.adventure.c
	$(CC) -Wall -ansi -c leejuic.adventure.c

leejuic.buildrooms: leejuic.buildrooms.o
	$(CC) -Wall -ansi -o leejuic.buildrooms leejuic.buildrooms.o

leejuic.adventure: leejuic.adventure.o
	$(CC) -lpthread -Wall -ansi -o leejuic.adventure leejuic.adventure.o

clean:
	rm *.o

cleanall: 
	rm -rf leejuic.rooms.* *.o

