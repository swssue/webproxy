# Makefile for Proxy Lab 
#
# You may modify this file any way you like (except for the handin
# rule). You instructor will type "make" on your specific Makefile to
# build your proxy from sources.

CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lpthread


csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) -c csapp.c

client: echoClient.o csapp.o
	$(CC) $(CFLAGS) -o client echoClient.o csapp.o

server: echoServer.o csapp.o echo.o
	$(CC) $(CFLAGS) -o server echoServer.o echo.o csapp.o  $(LDFLAGS)


clean:
	rm -f *~ *.o proxy core *.tar *.zip *.gzip *.bzip *.gz

