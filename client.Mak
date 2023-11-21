SRCS-y := main_client.cpp client.h command.h config.h

CC = g++
CFLAGS = -Wall -g -O0

client: client.o
	$(CC) $(CFLAGS) -o client client.o

client.o: $(SRCS-y)
	$(CC) $(CFLAGS) -c $(SRCS-y)

clean:
	rm -f client client.o
