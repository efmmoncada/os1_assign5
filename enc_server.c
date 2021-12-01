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

char *encypt(char *plaintext, char *key) {

}

int main(int argc, char *argv[]){
  char *key = NULL;
  char *plaintext = NULL;
  char *msg[250000] = {0};

  int connectionSocket, charsRead;
  char buffer[1001];
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

    printf("SERVER: Connected to client running at host %d port %d\n", 
          ntohs(clientAddress.sin_addr.s_addr),
          ntohs(clientAddress.sin_port));

    // Get the message from the client and display it
    memset(buffer, '\0', sizeof(buffer));
    // Read the client's message from the socket
    charsRead = recv(connectionSocket, buffer, 1001, 0); 
    if (charsRead < 0){
      error("ERROR reading from socket");
    }
    if (strcmp(buffer, "I am enc server") != 0) {
      charsRead = send(connectionSocket, "Connection refused", 18, 0);
      fprintf(stderr, "Rejected connection from dec server.\n");
      close(connectionSocket);
      continue;
    }
    // ! this cant stay for final submissoin
    printf("SERVER: I received this from the client: \"%s\"\n", buffer);
    send(connectionSocket, "Connection successful", 21, 0);

    while (1) {
      charsRead = recv(connectionSocket, buffer, sizeof(buffer), 0);
      printf("SERVER: I received this from the client: \"%s\"\n", buffer);

      if (strstr(buffer, "$") != NULL) {
        printf("%s\n", buffer);
        break;
      }
    }
    

    // Send a Success message back to the client
    charsRead = send(connectionSocket, "I am the server, and I got the whole message", 44, 0); 
    if (charsRead < 0){
      error("ERROR writing to socket");
    }
    // Close the connection socket for this client
    close(connectionSocket); 
  }
  // Close the listening socket
  close(listenSocket); 
  return 0;
}
