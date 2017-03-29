CC = gcc
CFLAGS = -m32

main: main.o fs.o disk.o
	$(CC) $(CFLAGS) -o main main.o fs.o disk.o

fs.o: fs.c
	$(CC) $(CFLAGS) -c fs.c

disk.o: disk.c
	$(CC) $(CFLAGS) -c disk.c

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

clean:
	$(RM) main main.o fs.o disk.o