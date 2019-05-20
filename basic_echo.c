#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h> 
#include <stdbool.h>

#define BUFFER_SIZE 1024
#define on_error(...) { fprintf(stderr, __VA_ARGS__); fflush(stderr); exit(1); }

int gameStarted = 0;
int numPlayers = 0;
int playerID[8];
int playerLife[8];

//  Checks if still connected... TODO
int stillConnected(int playerNum) {
  err = send(playerID[playerNum], '.', read, 0);
  if (err < 0) {
    playerID[playerNum] = 0;
    for (int i = playerNum; i < numPlayers - 1; i++) {
      playerID[i] = playerID[i+1];
      playerLife[i] = playerLife[i+1];
    }
    playerID[7] = 0;
    playerLife[7] = 0;
  }
}

//  Tries to connect to the game...
int tryConnect(int clientID) {
  if (gameStarted == 0) {
    if (numPlayers != 8) {
      playerID[numPlayers] = clientID;
      playerLife[numPlayers] = 5;
      numPlayers++;
      return 1;
    } else {
      return 0;
    }
  } else {
    return 0;
  }
}

int startGame() {
  int currentPlayers = 
}

void *echoInp (void *client) {
  int clientID = *((int *) client);
  char buf[BUFFER_SIZE];
  int read = 0;
  int connected = 0;
  while (1) {
      memset(buf, 0, sizeof(buf));
      read = recv(clientID, buf, BUFFER_SIZE, 0);

      if (!read) break;
      if (read < 0) on_error("Client read failed\n");
      
      printf("CLIENT %03d: %s\n", clientID, buf);      
    }
}

void *startServer (void *arg) {
  int port = 4000;
  int server_fd, client_fd, err;
  struct sockaddr_in server, client;
  char buf[BUFFER_SIZE];

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) on_error("Could not create socket\n");

  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  int opt_val = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);

  err = bind(server_fd, (struct sockaddr *) &server, sizeof(server));
  if (err < 0) on_error("Could not bind socket\n");

  err = listen(server_fd, 128);
  if (err < 0) on_error("Could not listen on socket\n");

  printf("Server is listening on %d\n", port);

  while (1) {
    socklen_t client_len = sizeof(client);
    client_fd = accept(server_fd, (struct sockaddr *) &client, &client_len);
    
    printf("Connected to client #%03d!\n", client_fd);

    if (client_fd < 0) on_error("Could not establish new connection\n");
    
    int *arg = malloc(sizeof(*arg));
    if ( arg == NULL ) {
        fprintf(stderr, "Couldn't allocate memory for thread arg.\n");
        exit(EXIT_FAILURE);
    }
    *arg = client_fd;
    pthread_t thread;
    pthread_create(&thread, 0, echoInp, arg);
  }
}

int main() {
  pthread_t thread;
  pthread_create(&thread, 0, startServer, NULL);
  while(1);
}