//  INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>


//  LISTEN CLIENT
//  Receives any input sent by a clientID, then stores it for access
void *listenClient(void *client_id) {
  
  int clientID = *((int *) client_id);
  char buf[BUFFER_SIZE];
  int read = 0;
  int connected = 0;
  int joined = 0;
  int timeout = 10;
  
  while (1) {
    memset(buf, 0, sizeof(buf));
    read = recv(clientID, buf, BUFFER_SIZE, 0);

    if (!read) break;
    if (read < 0) printf("Client read failed\n");

    printf("CLIENT %03d: %s\n", clientID, buf);

    if (joined == 1) {
      memcpy(clientInput[clientID], buf, strlen(buf)+1);
    } else {
      if (strcmp(buf, "INIT") == 0) {
        printf("Attempting to join game...\n");
        if (attemptJoin(clientID)) {
          joined == 1;
        }
      }
    }
  }
  
}

//  SERVER
//  Starts the server, and listens to the port
void *server(void *arg) {
  int server_id, client_id, err;
  struct sockaddr_in server, client;
  char buf[BUFFER_SIZE];

  server_id = socket(AF_INET, SOCK_STREAM, 0);
  if (server_id < 0) printf("Could not create socket\n");

  server.sin_family = AF_INET;
  server.sin_port = htons(4000);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  int opt_val = 1;
  setsockopt(server_id, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);

  err = bind(server_id, (struct sockaddr *) &server, sizeof(server));
  if (err < 0) printf("Could not bind socket\n");

  err = listen(server_id, 128);
  if (err < 0) printf("Could not listen on socket\n");

  printf("Server is listening on %d\n", 4000);

  while (1) {
    socklen_t client_len = sizeof(client);
    client_id = accept(server_id, (struct sockaddr *) &client, &client_len);
    
    printf("Connected to client #%03d!\n", client_id);

    if (client_id < 0) printf("Could not establish new connection\n");
    
    int *clientID = malloc(sizeof(*clientID));
    if ( clientID == NULL ) {
        printf("Couldn't allocate memory for thread arg.\n");
    }
    *clientID = client_id;
    pthread_t thread;
    pthread_create(&thread, 0, listenClient, clientID);
  }
}
