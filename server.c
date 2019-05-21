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

//  SERVER VARIABLES
#define PORT 4000
#define BUFFER_SIZE 1024
#define MAX_CLIENT_ID 32

//  PLAYING VARIABLES
#define NUMPLAYERS 8
#define MINPLAYERS 4

//  INITS
//    //  STATE VARIABLES
char  serverState[16];
char  clientInput[32][MAX_CLIENT_ID];
char  playerState[16][NUMPLAYERS];
char  playerInput[32][NUMPLAYERS];
int   playerID[NUMPLAYERS];
int   playerLife[NUMPLAYERS];
int   numPlayers = 0;
int   numRound = 0;
//    //  GAME VARIABLES
int   rollOne = 0;
int   rollTwo = 0;
int   rollSum = 0;

//  START
//  Starts the server in background and executes the game
int start() {
  
  pthread_t serverThread;
  pthread_create(&serverThread, NULL, server, NULL);
  
  while(1) {
    playGame();
  }
  
}

//  SERVER
//  Starts the server, and listens to the port
void *server(void *arg) {
  
  int server_id, client_id, err;
  struct sockaddr_in server, client;
  
  server_id = socket(AF_INET, SOCK_STREAM, 0);
  if (server_id < 0) {
    printf("Failed to create socket.\n");
    exit();
  }
  
  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  
  int optval = 1;
  setsockopt(server_id, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
  
  err = (server_id, (struct sockaddr *) &server, sizeof(server));
  if (err < 0) {
    printf("Failed to bind socket.\n");
    exit();
  }
  
  err = listen(server_id, 128);
  if (err < 0) {
    printf("Failed to listen on socket.\n");
    exit();
  }
  
  printf("Server started and listening on %d\n", PORT);
  
  while (1) {
    socklen_t client_len = sizeof(client);
    client_id = accept(server_id, (struct sockaddr *) &client, &client_len);

    if (client_id < 0) {
      printf("Could not establish a connection\n");
    } else {
      printf("Connected to client #%03d...\n", client_id);
      exit();
    }
    
    if (clientID < MAX_CLIENT_ID) {
      int *clientID = malloc(sizeof(*clientID));
      if ( clientID == NULL ) {
        printf("Failed to allocate memory to new client thread...");
        exit();
      }
      *clientID = client_id;
      pthread_t clientThread;
      pthread_create(&clientThread, 0, listen, clientID);
    }
  }
  
}

//  LISTEN
//  Receives any input sent by a clientID, then stores it for access
void *listen(void *client_id) {
  
  int clientID = *((int *) client_id);
  char buffer[BUFFER_SIZE];
  int read = 0;
  
  while (1) {
    memset(buffer, 0, sizeof(buffer));
    read = recv(clientID, buffer, BUFFER_SIZE, 0);
    
    if (!read) break;
    if (read < 0) {
      printf("Reading from client #%03d failed.\n", clientID);
    }
    strcpy(clientInput[clientID], buf)
  }
  
}

//  PLAY GAME
//  Starts the game, initialising all the game variables. Can be used to both play/replay the game
void playGame() {
  
  numPlayers = 0;
  clearStrArray(playerState);
  clearStrArray(playerInput);
  memset(playerID, 0, sizeof(playerID));
  memset(playerLife, 0, sizeof(playerLife));
  rollOne = 0;
  rollTwo = 0;
  rollSum = 0;
  strcpy(serverState, "LISTENING");
  
  sendAll("A new game has started, awaiting more players...\n")
  
  while (serverState == "LISTENING") {
    if (numPlayers > 0) {
      waitPlayers();
    }
  }
  
  sendAll("The game has started...\n")
  
  while (playRound());
  
  sendAll("The game has ended...\n")
  for (i = 0; i < numPlayers; i++) {
    if playerState[i] == "ELIMINATED" {
      send(playerID[i], "You were eliminated from the game, good luck next time!\n");
    } else if playerState[i] == "VICTORIOUS" {
      send(playerID[i], "You were victorious in this game, good job!\n");
    }
  }
  removeAllPlayers();
  
}

//  PLAY ROUND
//  Plays a round of the game, and calculates whether the game has ended or is still ongoing
int playRound() {
  
  sendAll("Round number %d has started, make your move within the next 5 seconds...\n", numRound);
  calculateRolls();
  
  inputClear();
  checkConnected();
  sleep(5);
  
  inputParse();
  calculateLives();
  calculateStates();
  checkConnected();
  
  int numAlive = 0;
  for (i = 0; i < numPlayers; i++) {
    if (playerState[numPlayers] == "ALIVE") {
      numAlive++;
    }
  }
  
  if (numAlive > 0) {
    return 1;
  } else {
    return 0;
  }
}

//  WAIT PLAYERS
//  Waits for the minimum amount of players to enter the game, then starts a countdown
int waitPlayers() {
  
}

//  INPUT CLEAR
//  Clears all the input and parsed strings
int inputClear() {}

//  INPUT PARSE
//  Reads all player inputs in the storage array (including malformed), and converts them to a usable form
int inputParse() {}

//  INPUT CHECK
//  Checks the input against the stored variables and returns a correct or wrong value
int checkAnswer() {}

//  CALCULATE ROLLS
//  Calculates the new dice rolls
void calculateRolls() {
  rollOne = rand() % 6 + 1;
  rollTwo = rand() % 6 + 1;
  rollSum = rollOne + rollTwo;
}

//  CALCULATE LIVES
//  Calculates the lives of all the players, whether they pass or fail
int calculateLives() {}

//  CALCULATESTATES
//  Calculates the states of all the players
int calculateStates() {}

//  CHECK CONNECTED
//  Checks if all the players are still connected. If not, they are removed from the player list
int checkConnected() {}

//  REMOVE PLAYER
//  Removes a specific player (given an index) from the game
int removePlayer(int playerIdx) {}

//  REMOVE ALL PLAYERS
//  Removes all the players from the game
int removeAllPlayers() {}

//  SEND
//  Sends a message to a client ID
int send() {}

//  SEND ALL
//  Sends a message to all connected clients
int sendAll() {}

//  SEND PLAYERS
//  Sends a message to all connected players
int sendPlayers() {}

//  SEND STATUS
//  Sends the statuses of each player
int sendStatus() {}

//  CLEAR STR ARRAY
//  Clears a string array and initialises it to empty values
int clearStrArray(*strArray) {
  for (i = 0; i < sizeof strArray; i++) {
    strcpy(strArray[i], '');
  }
}

//  MAIN
//  Executes the application when all functions have been loaded into memory 
int main() {start();}