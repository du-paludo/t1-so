flags = -Wall -std=c99
nome = conway

all: $(nome)

$(nome): main.o
	gcc -o $(nome) main.o $(flags)

main.o: main.c
	gcc -c main.c $(flags)

clean:
	rm -f *~ *.o

purge: clean
	rm -f $(nome)