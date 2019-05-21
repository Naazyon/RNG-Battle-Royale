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
int numOne = 0;
int numTwo = 0;
int sumNum = 0;
char[32] playerInput[8];
char[16] playerState[8];

// Calculates whether a life should be removed from a player...
void calculateLives() {
  numOne = generateRand(6);
  numTwo = generateRand(6);
  sumNum = numOne + numTwo;
  int i = 0;
  while (i < numPlayers) {
    if (stillConnected(playerID[i])) {
    if (playerState[i] == "PLAYING") {
      if !(check(playerInput[i]), numOne, numTwo, sumNum) {
          playerLife[i] = playerLife[i] - 1;
        }
      } else if {playerState[i] == "ELIMINATED"} {
      
      }
      i++;
    }
  }
}

// Changes the state of the player depending on life...
void changeStates() {
  int i = 0;
  int alive = 0;
  while (i < numPlayers) {
    if (playerLife[i] > 0) alive++; 
  }
  if (alive == 0) {
    
  }
}

//  Clears the current string
char[] clearStr(char[] inp){
  memset(inp, 0, sizeof(inp));
}

//  Checks if player still connected...
int stillConnected(int playerNum) {
  err = send(playerID[playerNum], '.', read, 0);
  if (err < 0) {
    for (int i = playerNum; i < numPlayers - 1; i++) {
      playerID[i] = playerID[i+1];
      playerLife[i] = playerLife[i+1];
      playerState[i] = playerState[i+1];
      playerInput[i] = playerInput[i+1];
    }
    playerID[7] = 0;
    playerLife[7] = 0;
    playerState[7] = clearStr(playerState[7]);
    playerInput[7] = clearStr(playerInput[7]);
    numPlayers--;
    return 0;
  }
  return 1;
}

//  Generates a random number between one and 'max' inclusive...
int generateRand(int max) {
  return rand() % 100 + 1;
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

//  Starts a 30 second timer to the start of the game...
void *startCountdown (void *arg) {
  int timer = 0;
  while(timer != 30) {
    sleep(1);
    timer +=1;
    if (numPlayers == 8) break;
  }
  if (numPlayers >= 4) {
    return 1;
  } else {
    return 0;
  }
}

//  Checks if the player passes or fails...
int check(char[] inp, int rollOne, int rollTwo, int rollSum) {
  printf(inp[7]);
  // "EVEN"
  if (inp[7] == "E") {
    if(rollOne != rollTwo && rollSum % 2 == 0) {
      return 1;
    } else {
      return 0;
    }
  }
  // "ODD"
  else if (inp[7] == "O") {
    if(rollSum > 5 && rollSum % 2 == 1) {
      return 1;
    } else {
      return 0;
    }
  }
  // "DOUB"
  else if (inp[7] == "D") {
    if (rollOne == rollTwo) {
      return 1;
    } else {
      return 0;
    }
  }
  // "CON"
  else if (inp[7] == "C") {
    int dig = inp[11] - "0";
    if (dig == rollOne || dig == rollTwo) {
      return 1;
    } else {
      return 0;
    }
  }
  // "INV"
  else {
    return 0;
  }

}

//  Echoes the client input
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

//  Starts a server connection and listens for any clients wanting to connect
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