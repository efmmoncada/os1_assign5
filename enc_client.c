#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()
#include <errno.h>

/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Prompt the user for input and send that input as a message to the server.
* 3. Print the message received from the server and exit the program.
*/

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber, 
                        char* hostname){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);

  // Get the DNS entry for this host name
  struct hostent* hostInfo = gethostbyname(hostname); 
  if (hostInfo == NULL) { 
    fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
    exit(0); 
  }
  // Copy the first IP address from the DNS entry to sin_addr.s_addr
  memcpy((char*) &address->sin_addr.s_addr, 
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}

void validate_files(char *key_filename, char *plain_filename, char **key_save, char **plaintext_save) {
  
  FILE* plain_file = fopen(plain_filename, "r");
  FILE* key_file = fopen(key_filename, "r");

  // * error if files cant be opened
  if (plain_file == NULL || key_file == NULL) {
    fprintf(stderr, "Error could not open file\n");
    exit(1);
  }
  
  // * get files and extract contents
  char *key = NULL;
  size_t key_b_size = 0;
  char *plaintext = NULL;
  size_t plain_b_size = 0;

  getline(&plaintext, &plain_b_size, plain_file);
  getline(&key, &key_b_size, key_file);
  plaintext[strlen(plaintext) - 1] = '\0';  // remove newlines
  key[strlen(key) - 1] = '\0';

  // * check for bad chars in key and plaintext
  for (int i = 0; i < strlen(key); i++) {
    if (key[i] != 32 && (key[i] < 65 || key[i] > 90)) {
      fprintf(stderr, "ERROR: Key contains bad characters.\n");
      exit(1);
    }
  }

  for (int i = 0; i < strlen(plaintext); i++) {
    if (plaintext[i] != 32 && (plaintext[i] < 65 || plaintext[i] > 90)) {
      fprintf(stderr, "ERROR: Plaintext contains bad characters.\n");
      exit(1);
    }
  }

  // * check for key len >= plaintext
  if (strlen(key) < strlen(plaintext)) {
    fprintf(stderr, "ERROR: Key is shorter than plaintext.\n");
    exit(1);
  }

  *key_save = calloc(strlen(key) + 1, sizeof(char));
  *plaintext_save = calloc(strlen(plaintext) + 1, sizeof(char));
  strcpy(*key_save, key);
  strcpy(*plaintext_save, plaintext);

  free(key);
  free(plaintext);
}


int main(int argc, char *argv[]) {
  int socketFD, portNumber, charsWritten, charsRead;
  struct sockaddr_in serverAddress;

  char *key = NULL;
  char *plaintext = NULL;
  char buffer[101] = {0};

  // * Check usage & args
  if (argc != 4) { 
    fprintf(stderr,"USAGE: %s <plaintext> <key> <port>\n", argv[0]); 
    exit(0);
  }

  portNumber = atoi(argv[3]);

  validate_files(argv[2], argv[1], &key, &plaintext);


  // Create a socket
  socketFD = socket(AF_INET, SOCK_STREAM, 0); 
  if (socketFD < 0){
    fprintf(stderr, "CLIENT: ERROR opening socket");
    exit(1);
  }

   // Set up the server address struct
  setupAddressStruct(&serverAddress, portNumber, "localhost");

  // Connect to server
  if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
    fprintf(stderr, "CLIENT: ERROR connecting");
    exit(1);
  }

  // Send message to server
  // Write to the server
  charsWritten = send(socketFD, "I am enc server", 16, 0);
  charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
  if (strcmp(buffer, "Connection refused") == 0) {
    fprintf(stderr, "Connection rejected by server\n");
    exit(2);
  }
  
  // * msg format: <key>#<plaintext>
  // * allocating space for key + '#' + plaintext + '$' + '\0'
  char *msg = calloc(strlen(key) + strlen(plaintext) + 3, sizeof(char));
  memcpy(msg, key, strlen(key));
  memcpy(msg + strlen(key), "#", 1);
  memcpy(msg + strlen(key) + 1, plaintext, strlen(plaintext));
  memcpy(msg + strlen(key) + 1 + strlen(plaintext), "$", 1);
  
  unsigned int total_written = 0;
  
  const int chunk_size = 100;
  
  while (1) {
    if (total_written == strlen(msg)) {
      break;
    }
    // ! TODO: this block which handles case where remaining is less than chunk size is not working correclty. 
    if (strlen(msg) - total_written < chunk_size) {
      memcpy(buffer, msg + total_written, strlen(msg) - total_written);
      charsWritten = send(socketFD, buffer, strlen(buffer), 0);
      total_written += charsWritten;
    }
    else {
      memcpy(buffer, msg + total_written, chunk_size);
      charsWritten = send(socketFD, buffer, strlen(buffer), 0);
      if (charsWritten < 0) {
        fprintf(stderr, "CLIENT: ERROR writing to socket, retrying\n");
        perror(NULL);
        continue;
      }
      if (charsWritten < strlen(buffer)) {
        fprintf(stderr, "CLIENT: WARNING: Not all data written to socket!\n");
        // TODO: handle error
      }
      total_written += charsWritten;
    }
  }
  
  

    // // Get return message from server
    // // Clear out the buffer again for reuse
    // memset(buffer, '\0', sizeof(buffer));
    // // Read data from the socket, leaving \0 at end
    // charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); 
    // if (charsRead < 0){
    //   fprintf(stderr, "CLIENT: ERROR reading from socket");
    // }
    // printf("CLIENT: I received this from the server: \"%s\"\n", buffer);

    // Close the socket
    close(socketFD); 
    return 0;
}