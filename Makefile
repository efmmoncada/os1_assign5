CC = gcc
CFLAGS = -g -Wall --std=gnu99

all: keygen enc_client enc_server dec_client dec_server

keygen: keygen.c
	$(CC) $(CFLAGS) -o keygen keygen.c 

enc_client: enc_client.c
	$(CC) $(CFLAGS) -o enc_client enc_client.c 

enc_server: enc_server.c
	$(CC) $(CFLAGS) -o enc_server enc_server.c

dec_client: dec_client.c
	$(CC) $(CFLAGS) -o dec_client dec_client.c

dec_server: dec_server.c
	$(CC) $(CFLAGS) -o dec_server dec_server.c

clean:
	rm -rf enc_client enc_server dec_client dec_server keygen

.PHONY: 
	clean

