CC = gcc
CFLAGS = -g -Wall -pthread
all: server client 

server: server.o  common.o reentrant.o
	$(CC) $(CFLAGS) -o $@ $^ -lssl -lcrypto
client : client.o common.o reentrant.o 
	$(CC) $(CFLAGS) -o $@ $^ -lssl -lcrypto
client1 : client1.o common.o reentrant.o 
	$(CC) $(CFLAGS) -o $@ $^ -lssl -lcrypto
client.o: client.c
	$(CC) -c -o $@ $^
server.o: server.c
	$(CC) -c -o $@ $^
common.o: common.c
	$(CC) $(CFLAGS) -c -o $@ $^
reentrant.o: reentrant.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -f *.o
	rm -f $(TARGET)
