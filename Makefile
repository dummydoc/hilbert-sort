CC = gcc
FLAGS = -Wall -g -c

all: convptsb hilbert

convptsb: convptsb.o
	$(CC) convptsb.o -o convptsb

hilbert: hilbert.o
	$(CC) -D_GNU_SOURCE hilbert.o -o hilbert -lGL -lGLU -lglut

convptsb.o: convptsb.c 
	$(CC) $(FLAGS) convptsb.c

hilbert.o: hilbert.c hilbert.h
	$(CC) $(FLAGS) hilbert.c

clean:
	rm *.o
