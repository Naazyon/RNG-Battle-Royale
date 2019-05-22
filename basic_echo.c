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
#define NUMPLAYERS 8
#define MINPLAYERS 4

int numPlayers = 0;
int numOne = 0;
int numTwo = 0;
int sumNum = 0;
int stillAlive = 0;
int gameStarted = 0;
int numConnections = 0;
char serverState[16] = "LISTENING";
int playerID[NUMPLAYERS];
int numWinners[NUMPLAYERS];
int winnerID[NUMPLAYERS];
int playerLife[NUMPLAYERS];
char playerInput[32][NUMPLAYERS];
char playerOutput[32][NUMPLAYERS];
char playerState[32][NUMPLAYERS];

// Checks if all players are still connected...
void checkConnected() {
  
}

// Starts a round...
void playRound() {
  while(stillAlive) {
    checkConnected();
    // sendAll(Round details)
    // sendEach(Specific details)
    // clear input strings
    // clear output strings
    strcpy(serverState, "ROUND");
    sleep(5);
    calculateLives();
    // sendEach(Specific details)
  }
  // disconnectPlayers()
  // end Game
  // re-initialize
}

// Listens and if people start to connect, starts the countdown
void listenAndStart() {
  // Server listening...
  while(serverState == "LISTENING") {
    sleep(1);
    if(numConnections != 0) {
      strcpy(serverState, "WAITING");
      if(startCountdown()){
        strcpy(serverState, "CALCULATING");
        break;
      } else {
        // breakConnections(); TODO 
        strcpy(serverState, "LISTENING");
      }
    }
  }
}

// Calculates whether a life should be removed from a player...
void calculateLives() {
  numOne = generateRand(6);
  numTwo = generateRand(6);
  sumNum = numOne + numTwo;
  int i = 0;
  while (i < numPlayers) {
    if (stillConnected(playerID[i])) {
    if (playerState[i] == "PLAYING") {
      if (!check(playerInput[i]), numOne, numTwo, sumNum) {
          playerLife[i] = playerLife[i] - 1;
        }
      }
      i++;
    }
  }
}

// Changes the state of the player depending on life...
void changeStates() {
  int i = 0;
  int alive = 0;
  char stateStr[16];
  strcpy(stateStr, "ELIMINATED");
  char sendStr[8];
  strcpy(sendStr, "ELIM");
  while (i < numPlayers) {
    if (playerLife[i] > 0) alive++;
    i++;
  }
  if (alive == 0) {
    strcpy(stateStr, "VICTORIOUS");
    strcpy(sendStr, "VICT");
  }
  while(i < numPlayers) {
    if (playerState[i] == "PLAYING" && playerLife[i] == 0) {
      strcpy(playerState[i], stateStr);
      strcpy(playerOutput[i], sendStr);
    }
    i++;
  }
}

//  Checks if player still connected...
int stillConnected(int playerNum) {
  int err = send(playerID[playerNum], '.', read, 0);
  if (err < 0) {
    for (int i = playerNum; i < numPlayers - 1; i++) {
      playerID[i] = playerID[i+1];
      playerLife[i] = playerLife[i+1];
      strcpy(playerState[i], playerState[i+1]);
      strcpy(playerInput[i], playerInput[i+1]);
    }
    playerID[NUMPLAYERS-1] = 0;
    playerLife[NUMPLAYERS-1] = 0;
    strcpy(playerState[NUMPLAYERS-1], "");
    strcpy(playerInput[NUMPLAYERS-1], "");
    numPlayers--;
    return 0;
  }
  return 1;
}

//  Generates a random number between one and 'max' inclusive...
int generateRand(int max) {
  return rand() % max + 1;
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
void startCountdown () {
  int timer = 0;
  while(timer != 30) {
    sleep(1);
    timer +=1;
    if (numPlayers == 8) break;
  }
  if (numPlayers >= MINPLAYERS) {
    return 1;
  } else {
    return 0;
  }
}

//  Checks if the player passes or fails...
int check(char inp[], int rollOne, int rollTwo, int rollSum) {
  printf(inp[8]);
  // "EVEN"
  if (inp[8] == "E") {
    if(rollOne != rollTwo && rollSum % 2 == 0) {
      return 1;
    } else {
      return 0;
    }
  }
  // "ODD"
  else if (inp[8] == "O") {
    if(rollSum > 5 && rollSum % 2 == 1) {
      return 1;
    } else {
      return 0;
    }
  }
  // "DOUB"
  else if (inp[8] == "D") {
    if (rollOne == rollTwo) {
      return 1;
    } else {
      return 0;
    }
  }
  // "CON"
  else if (inp[8] == "C") {
    char numSubmitted = inp[12];
    char zero = "0";
    int dig = numSubmitted -zero;
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