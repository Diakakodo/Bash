tesh: tesh.o
	gcc -Wall -std=gnu99 -o tesh tesh.o -ldl

tesh.o: tesh.c
	gcc -Wall -std=gnu99 -c tesh.c -o tesh.o
