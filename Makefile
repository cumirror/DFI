test: main.o acsmx.o partition.o 
	gcc -o test main.o acsmx.o partition.o

main.o: main.c acsmx.h comm.h
	gcc -c main.c
acsmx.o: acsmx.c partition.h comm.h
	gcc -c acsmx.c
partition.o: partition.c comm.h
	gcc -c partition.c

clean:
	rm *.o test
