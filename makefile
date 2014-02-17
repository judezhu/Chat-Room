#makefile4
CC=g++ -std=c++0x 
CFLAGS=-lpthread -I.
OBJ = server.o 

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

server: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f Server
	
