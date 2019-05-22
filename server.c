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
#define MAX_CONNECTIONS 32

//  PLAYING VARIABLES
#define MAXPLAYERS 8
#define MINPLAYERS 4
#define START_LIVES 5

//  INITS
//    //  SERVER VARIABLES
char  serverState[16];
char  clientInput[32][MAX_CONNECTIONS+12];
int   connectionID[MAX_CONNECTIONS];
int   numConnections = 0;
//    //  GAME VARIABLES
char  playerState[16][MAXPLAYERS];
char  playerInput[32][MAXPLAYERS];
int   playerID[MAXPLAYERS];
int   playerLife[MAXPLAYERS];
int   numPlayers = 0;
int   numRound = 0;
int   rollOne = 0;
int   rollTwo = 0;
int   rollSum = 0;


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
  server.sin_port = htons(PORT);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  int opt_val = 1;
  setsockopt(server_id, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);

  err = bind(server_id, (struct sockaddr *) &server, sizeof(server));
  if (err < 0) printf("Could not bind socket\n");

  err = listen(server_id, 128);
  if (err < 0) printf("Could not listen on socket\n");

  printf("Server is listening on %d\n", PORT);

  while (1) {
    socklen_t client_len = sizeof(client);
    client_id = accept(server_id, (struct sockaddr *) &client, &client_len);
    
    printf("Connected to client #%03d!\n", client_id);

    if (client_id < 0) printf("Could not establish new connection\n");
    
    int *clientID = malloc(sizeof(*clientID));
    if ( clientID == NULL ) {
        printf("Couldn't allocate memory for thread arg.\n");
        exit(EXIT_FAILURE);
    }
    *clientID = client_id;
    pthread_t thread;
    pthread_create(&thread, 0, listenClient, clientID);
  }
}

//  START
//  Starts the server in background and executes the game
int start() {
  
  pthread_t thread;
  pthread_create(&thread, NULL, server, NULL);
  
  printf("Server created...\n");
  
  while(1) {
    playGame();
  }
  
}

//  PLAY GAME
//  Starts the game, initialising all the game variables. Can be used to both play/replay the game
void playGame() {
  
  printf("Initialising variables...\n");
  
  numPlayers = 0;
  clearStrArray(playerState);
  clearStrArray(playerInput);
  memset(playerID, 0, sizeof playerID);
  memset(playerLife, 0, sizeof playerLife);
  rollOne = 0;
  rollTwo = 0;
  rollSum = 0;
  strcpy(serverState, "LISTENING");
  
  sendAll("A new game has started, awaiting more players...\n");
  
  while (strcmp(serverState, "LISTENING") == 0) {
    if (numPlayers > 0) {
      if (waitPlayers()) {
        strcpy(serverState, "PLAYING");
        sendPlayers("START,%02d,%02d", numPlayers, START_LIVES);
      } else {
        sendPlayers("CANCEL");
        removeAllPlayers();
      }
    }
  }
  
  sendAll("The game has started...\n");
  
  while (calculateAlive()) playRound();
  
  sendAll("The game has ended...\n");
  for (int i = 0; i < numPlayers; i++) {
    if (playerState[i] == "ELIMINATED") {
      sendMsg(playerID[i], "You were eliminated from the game, good luck next time!\n");
    } else if (playerState[i] == "VICTORIOUS") {
      sendMsg(playerID[i], "You were victorious in this game, congratulations!\n");
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
  calculateAlive();
  calculateStates();
  
  int numAlive = 0;
  for (int i = 0; i < numPlayers; i++) {
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
  
  int count = 0;
  while (count < 30) {
    sleep(1);
    if (numPlayers == MAXPLAYERS) {
      return 1;
    }
    count++;
  }
  
  if (numPlayers >= 4) {
    return 1;
  } else {
    return 0;
  }
  
}

//  INPUT CLEAR
//  Clears all the input and parsed strings
int inputClear() {
  clearStrArray(clientInput);
  clearStrArray(playerInput);
}

//  INPUT PARSE
//  Reads all player inputs in the storage array and moves them to the playerInput
int inputParse() {
  int currentID = 0;
  char rawInput[32];
  for (int i = 0; i < numPlayers; i++) {
    currentID = playerID[i];
    strcpy(rawInput, clientInput[currentID]);
  }
}

//  INPUT CHECK
//  Checks the input against the stored variables and returns a correct or wrong value
int checkAnswer(char inp[]) {
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
    char numSubmitted = inp[11];
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

//  CALCULATE ROLLS
//  Calculates the new dice rolls
void calculateRolls() {
  rollOne = rand() % 6 + 1;
  rollTwo = rand() % 6 + 1;
  rollSum = rollOne + rollTwo;
}

//  CALCULATE LIVES
//  Calculates the lives of all the players, whether they pass or fail
void calculateLives() {
  for (int i = 0; i < numPlayers; i++) {
    if (playerState[i] == "ALIVE") {
      if (checkAnswer(playerInput[i])) {
        sendMsg(playerID[i], ("%03d,PASS\n", playerID[i]));
      } else {
        sendMsg(playerID[i], ("%03d,FAIL\n", playerID[i]));
        playerLife[i] = playerLife[i]-1;
      }
    }
  }
}

//  CALCULATE ALIVE
//  Check if anyone is alive
int calculateAlive() {
  int alive = 0;
  for (int i = 0; i < numPlayers; i++) {
    if (playerLife[i] > 0) alive++;
  }
  if (alive) return alive;
  else return 0;
}

//  CALCULATE STATES
//  Calculates the states of all the players
void calculateStates() {
  int stillAlive = calculateAlive();
  for (int i = 0; i < numPlayers; i++) {
    if (playerState[i] == "ALIVE") {
      if (playerLife[i] == 0) {
        if (stillAlive) {
          playerState[i] == "ELIMINATED";
          sendMsg(playerID[i], ("%03d,ELIM\n", playerID[i]));
        } else {
          playerState[i] == "VICTORIOUS";
          sendMsg(playerID[i], ("VICT\n", playerID[i]));
        }
      }
    }
  }
}

//  CHECK CONNECTED
//  Checks if all the players are still connected. If not, they are removed from the player list
void checkConnected() {
  int err;
  for (int i = 0; i < numPlayers; i++) {
    err = send(playerID[i], '\0', read, 0);
    if (err < 0) removePlayer(i);
  }
}

//  REMOVE PLAYER
//  Removes a specific player (given an index) from the game
int removePlayer(int playerIdx) {
  for (int i = playerIdx; i < numPlayers - 1; i++) {
      playerID[i] = playerID[i+1];
      playerLife[i] = playerLife[i+1];
      strcpy(playerState[i], playerState[i+1]);
      strcpy(playerInput[i], playerInput[i+1]);
    }
    playerID[MAXPLAYERS-1] = 0;
    playerLife[MAXPLAYERS-1] = 0;
    strcpy(playerState[MAXPLAYERS-1], "");
    strcpy(playerInput[MAXPLAYERS-1], "");
    numPlayers--;
}

//  REMOVE ALL PLAYERS
//  Removes all the players from the game
int removeAllPlayers() {
  for (int i = numPlayers - 1; i >= 0; i--) {
    removePlayer(i);
  }
}

//  SEND MSG
//  Sends a message to a client ID
int sendMsg(int clientID, char message[]) {
  printf("Sending message to %03d", clientID);
  int err = send(clientID, message, sizeof message, 0);
  if (err < 0) {
    printf("Sending message %s to %d failed...\n", message, clientID);
  }
}

//  SEND ALL
//  Sends a message to all connected clients
int sendAll(char message[]) {
  printf("Attempting to send all...", message);
  for (int i = 0; i < numConnections; i++) {
    send(connectionID[i], message, sizeof message, 0);
  }
  printf("Sent.\n", message);
}

//  SEND PLAYERS
//  Sends a message to all connected players
int sendPlayers(char message[]) {
  for (int i = 0; i < numPlayers; i++) {
    sendMsg(playerID[i], message);   
  }
}

//  SEND STATUS
//  Sends the statuses of each player
int sendStatus() {
  int currentID;
  sendAll(("The dice rolls from the previous round were %d and %d, with a maximum value of %d.\n", rollOne, rollTwo, rollSum));
  for (int i = 0; i < numPlayers; i++) {
    currentID = playerID[i];
    sendMsg(currentID, ("Your current state is %s.\n", playerState[i]));
    sendMsg(currentID, ("Your current lives are %02d.\n", playerLife[i]));
  }
  sendAll(("There are currently %d players still alive.\n", calculateAlive()));
}

//  CLEAR STR ARRAY
//  Clears a string array and initialises it to empty values
int clearStrArray(char strArray[][32]) {
  for (int i = 0; i < sizeof strArray; i++) {
    memcpy (strArray[i], "", strlen("")+1 );
  }
}

//  ATTEMPT JOIN
//  Attempts to join the game
int attemptJoin(int clientID) {
  printf("Attempting to join...\n");
  if (strcmp(serverState, "LISTENING") == 0) {
    printf("Server state is listening...\n");
    if (numPlayers != MAXPLAYERS) {
      printf("Room available to join...\n");
      sendAll(("Client %d connected to the game, there are now %d number of players...", clientID, numPlayers));
      playerID[numPlayers] *= clientID;
      playerLife[numPlayers] *= START_LIVES;
      numPlayers++;
      printf("Welcoming client...");
      sendMsg(clientID, ("WELCOME,%03d",clientID));
      printf("Client welcomed.n");
      return 1;
    }
  }
  return 0;
}

//  MAIN
//  Executes the application when all functions have been loaded into memory 
int main() {start();}