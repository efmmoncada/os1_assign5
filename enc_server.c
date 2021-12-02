#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Error function used for reporting issues
void error(const char *msg) {
  fprintf(stderr, msg);
  exit(1);
} 

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);
  // Allow a client at any address to connect to this server
  address->sin_addr.s_addr = INADDR_ANY;
}


char *get_ciphertext(char *plaintext, char *key) {
  char *ciphertext = calloc(strlen(plaintext) + 2, sizeof(char));
  memcpy(ciphertext, plaintext, sizeof(plaintext));

  for (int i = 0; i < strlen(plaintext); i++) {
    ciphertext[i] = ((ciphertext[i] + key[i]) % 27) + 64;
    if (ciphertext[i] == '@') ciphertext[i] = ' ';
  }
  ciphertext[strlen(plaintext) - 1] = '$';
  return ciphertext;
}

int main(int argc, char *argv[]){
  int connectionSocket, charsRead;
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);

  // Check usage & args
  if (argc < 2) { 
    fprintf(stderr,"USAGE: %s port\n", argv[0]); 
    exit(1);
  } 
  
  // Create the socket that will listen for connections
  int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSocket < 0) {
    error("ERROR opening socket");
  }

  // Set up the address struct for the server socket
  setupAddressStruct(&serverAddress, atoi(argv[1]));

  // Associate the socket to the port
  if (bind(listenSocket, 
          (struct sockaddr *)&serverAddress, 
          sizeof(serverAddress)) < 0){
    error("ERROR on binding");
  }

  // Start listening for connetions. Allow up to 5 connections to queue up
  listen(listenSocket, 5); 
  
  // Accept a connection, blocking if one is not available until one connects
  while(1) {
    // Accept the connection request which creates a connection socket
    connectionSocket = accept(listenSocket,(struct sockaddr *)&clientAddress, &sizeOfClientInfo); 
    if (connectionSocket < 0){
      error("ERROR on accept");
    }

    pid_t id = fork();
    if (id > 0) {
      close(connectionSocket);
      continue;
    }

    char msg[250000] = {0};
    memset(msg, '\0', sizeof(msg));

    // Get the message from the client and display it
    // Read the client's message from the socket
    charsRead = recv(connectionSocket, msg, sizeof(msg) - 1, 0); 
    if (charsRead < 0){
      error("ERROR reading from socket");
    }
    if (strcmp(msg, "I am enc server") != 0) {
      send(connectionSocket, "Connection refused", 18, 0);
      fprintf(stderr, "Rejected connection from dec server.\n");
      close(connectionSocket);
      continue;
    }

    send(connectionSocket, "Connection successful", 21, 0);

    unsigned int total_read = 0;
    while (1) {
      charsRead = recv(connectionSocket, msg + total_read, sizeof(msg) - 1 - total_read, 0);
      total_read += charsRead;
      //printf("SERVER: I received this from the client: \"%s\"\n", msg);
      if (strstr(msg, "$") != NULL) {
        break;
      }
    }

    char *key = strtok(msg, "#");
    char *plaintext = strtok(NULL, "$");

    char *ciphertext = get_ciphertext(plaintext, key);


    unsigned int total_written = 0;
    while (1) {
      if (total_written >= strlen(ciphertext)) {
        break;
      }
      total_written = send(connectionSocket, ciphertext + total_written, strlen(ciphertext) - total_written, 0);
    }
  
    // Close the connection socket for this client
    close(connectionSocket); 
  }

  // Close the listening socket
  close(listenSocket); 
  return 0;
}
