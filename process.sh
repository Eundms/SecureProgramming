#!/bin/bash
function printEcho {
	echo ">	$1"
}
printEcho "gcc -g -Wall -pthread -c reentrant.c -o reentrant.o"
gcc -g -Wall -pthread -c reentrant.c -o reentrant.o

printEcho "gcc -g -Wall -pthread -c common.c -o common.o"
gcc -g -Wall -pthread -c common.c -o common.o

printEcho "gcc -g -Wall -pthread -c client.c -o client.o"
gcc -g -Wall -pthread -c client.c -o client.o

printEcho "gcc -g -Wall -pthread -c client1.c -o client1.o"
gcc -g -Wall -pthread -c client1.c -o client1.o

printEcho "gcc -g -Wall -pthread -c server.c -o server.o"
gcc -g -Wall -pthread -c server.c -o server.o

printEcho "gcc -g -Wall -pthread -o client client.o common.o reentrant.o -lssl -lcrypto"
gcc -g -Wall -pthread -o client client.o common.o reentrant.o -lssl -lcrypto

printEcho "gcc -g -Wall -pthread -o client1 client1.o common.o reentrant.o -lssl -lcrypto"
gcc -g -Wall -pthread -o client1 client1.o common.o reentrant.o -lssl -lcrypto

printEcho "gcc -g -Wall -pthread -o server server.o common.o reentrant.o -lssl -lcrypto"
gcc -g -Wall -pthread -o server server.o common.o reentrant.o -lssl -lcrypto 
