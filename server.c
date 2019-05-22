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
#include <stdarg.h>

//  SERVER VARIABLES
#define PORT 4000
#define BUFFER_SIZE 1024
#define MAX_CONNECTIONS 32

//  PLAYING VARIABLES
#define MAX_PLAYERS 8
#define MIN_PLAYERS 2
#define MAX_LIVES 5

//  INITS
//    //  SERVER VARIABLES
int   clientInputs[MAX_CONNECTIONS];
int   clientOutputs[MAX_CONNECTIONS];
int   clientLives[MAX_CONNECTIONS];  //  1,2,3,4,5 < ALIVE | 0 < ELIMINATED / NOT PLAYING | -1 < VICTORIOUS
int   clientIDs[MAX_CONNECTIONS];
int   numClients = 0;
//    //  GAME VARIABLES
int   playerNums[MAX_PLAYERS];
int   numPlayers = 0;
int   rollOne = 0;
int   rollTwo = 0;
int   rollSum = 0;


//  LISTEN CLIENT
//  Receives any input sent by a client, then stores it for access
void *listenClient(void *client_connection) {
  
  int connectionNum = *((int *) client_connection);
  int clientID = clientIDs[connectionNum];
  char buffer[BUFFER_SIZE];
  char outBuf[BUFFER_SIZE];
  int read = 0;
  int outCode = 0;
  
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 100000; // 1/10th of a second...
  setsockopt(clientID, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
  
  while (1) {
    memset(buffer, 0, sizeof(buffer));
    read = recv(clientID, buffer, sizeof(buffer), 0);   //  Read with a timeout...
    if (read > 0) {   // If read...
      if (buffer[0] == "I") {
        //  INIT
        clientInputs[connectionNum] = 1;
      } else if (buffer[8] == "E") {
        //  EVEN
        clientInputs[connectionNum] = 2;
      } else if (buffer[8] == "O") {
        //  ODD
        clientInputs[connectionNum] = 3;
      } else if (buffer[8] == "D") {
        //  DOUBLE
        clientInputs[connectionNum] = 4;
      } else if (buffer[12] == "1") {
        //  CONTAINS 1
        clientInputs[connectionNum] = 5;
      } else if (buffer[12] == "2") {
        //  CONTAINS 2
        clientInputs[connectionNum] = 6;
      } else if (buffer[12] == "3") {
        //  CONTAINS 3
        clientInputs[connectionNum] = 7;
      } else if (buffer[12] == "4") {
        //  CONTAINS 4
        clientInputs[connectionNum] = 8;
      } else if (buffer[12] == "5") {
        //  CONTAINS 5
        clientInputs[connectionNum] = 9;
      } else if (buffer[12] == "6") {
        //  CONTAINS 6
        clientInputs[connectionNum] = 10;
      } else {
        //  UNKNOWN
        clientInputs[connectionNum] = 0;
      }
    }
    
    outCode = clientOutputs[connectionNum];
    if (outCode > 0) {    //  If message wanted to send...
      memset(outBuf, 0, sizeof(outBuf));
      
      if (outCode == 1) {
        sprintf(outBuf, "WELCOME,%03d", clientID);
        send(clientID, outBuf, sizeof outBuf, 0);
        
      } else if (outCode == 2) {
        sprintf(outBuf, "START,%02d,%02d", numPlayers, MAX_LIVES);
        send(clientID, outBuf, sizeof outBuf, 0);
        
      } else if (outCode == 3) {
        sprintf(outBuf, "%03d,PASS", clientID);
        send(clientID, outBuf, sizeof outBuf, 0);
        
      } else if (outCode == 4) {
        sprintf(outBuf, "%03d,FAIL", clientID);
        send(clientID, outBuf, sizeof outBuf, 0);
        
      } else if (outCode == 5) {
        sprintf(outBuf, "%03d,ELIM", clientID);
        send(clientID, outBuf, sizeof outBuf, 0);
        
      } else if (outCode == 6) {
        sprintf(outBuf, "VICT", clientID);
        send(clientID, outBuf, sizeof outBuf, 0);
        
      } else if (outCode == 7) {
        sprintf(outBuf, "REJECT", clientID);
        send(clientID, outBuf, sizeof outBuf, 0);
        
      } else if (outCode == 8) {
        sprintf(outBuf, "CANCEL", clientID);
        send(clientID, outBuf, sizeof outBuf, 0);
      }
      
      clientOutputs[connectionNum] = 0;
    }
  }
  
}

//  SERVER
//  Starts the server, and listens to the port
void *server(void *arg) {
  int server_id, client_id, err;
  struct sockaddr_in server, client;

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
    if (numClients == MAX_CONNECTIONS) {
      while(1);
    }
    socklen_t client_len = sizeof(client);
    client_id = accept(server_id, (struct sockaddr *) &client, &client_len);
    
    printf("Connected to client #%03d!\n", client_id);

    if (client_id < 0) printf("Could not establish new connection\n");
    
    clientIDs[numClients] = client_id;
    
    int *connectionNum = malloc(sizeof(*connectionNum));
    if ( connectionNum == NULL ) {
        printf("Couldn't allocate memory for thread arg.\n");
    }
    *connectionNum = numClients;
    pthread_t thread;
    pthread_create(&thread, 0, listenClient, connectionNum);
    numClients++;
  }
}

//  START
//  Starts the server in background and starts the game
void start() {
  
  pthread_t thread;
  pthread_create(&thread, NULL, server, NULL);
  
  printf("Server created...\n");
  
  while(1) {
    playGame();
  }
  
}

//  GET INPUT
//  Gets the input from a connection and clears it 
int getInput(int connectionNum) {
  int inp = 0;
  inp = clientInputs[connectionNum];
  clientInputs[connectionNum] = 0;
  return inp;
} 

//  SEND PLAYERS
//  Sends a message to all players
void sendPlayers(int message) {
  for (int i = 0; i < numPlayers; i++) {
    clientOutputs[playerNums[i]] = message;
  }
}

//  SEND ALL
//  Sends a message to all connections
void sendAll(char message[]) {
  for (int i = 0; i < numClients; i++) {
    clientOutputs[i] = message;
  }
}

//  PLAY GAME
//  Starts the game, initialising all the game variables. Can be used to both play/replay the game
void playGame() {
  
  memset(playerNums, 0, MAX_PLAYERS);
  numPlayers = 0;
  rollOne = 0;
  rollTwo = 0;
  rollSum = 0;
  printf("Initialised all variables..\n");
  
  int count = 0;
  
  while(1) {  //  Wait 10-15 seconds for people to join...
    sleep(1);
    count++;
    for (int conNum = 0; conNum < numClients; conNum++) {
      if (clientLives[conNum] < 1) { //  If not connected...
        if (getInput(conNum) == 1) {
          playerNums[numPlayers] = conNum;
          clientLives[conNum] = MAX_LIVES;
          numPlayers++;
          clientOutputs[conNum] = 1;
          printf("Client %03d joined...\n", clientIDs[conNum]);
        }
      }
    }
    if (numPlayers == MAX_PLAYERS || count == 10) {
      if(numPlayers >= MIN_PLAYERS) { //  If enough, start the game...
        printf("Game started...\n");
        sendPlayers(2);
      } else {  // Else, cancel...
        sendPlayers(8);
        removeAllPlayers();
        count = 0;
      }
    }
  }
  
  while(playRound()); //  Play round until everyone is dead...
  
  printf("The game has ended...\n");
  removeAllPlayers();
}

//  PLAY ROUND
//  Plays a round of the game, and calculates whether the game has ended or is still ongoing
int playRound() {
  calculateRolls();
  sleep(5);
  if(calculateLives()) return 1;
  else return 0;
}

//  CHECK ANSWER
//  Checks the input against the stored variables and returns a correct or wrong value
int checkAnswer(int conNum) {
  // "EVEN"
  int currentInp = getInput(conNum);
  if (currentInp == 2) {
    if(rollOne != rollTwo && rollSum % 2 == 0) {
      return 1;
    } else {
      return 0;
    }
  }
  // "ODD"
  else if (currentInp == 3) {
    if(rollSum > 5 && rollSum % 2 == 1) {
      return 1;
    } else {
      return 0;
    }
  }
  // "DOUBLE"
  else if (currentInp == 4) {
    if (rollOne == rollTwo) {
      return 1;
    } else {
      return 0;
    }
  }
  // "CONTAINS X"
  else if (currentInp > 4 && currentInp <= 10) {
    int digit = currentInp - 4;
    if (digit == rollOne || digit == rollTwo) {
      return 1;
    } else {
      return 0;
    }
  }
  // "INVALID"
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
  int stillAlive = 0;
  for (int i = 0; i < numPlayers; i++) {  //  Updates the lives of the players depending on a pass or fail
    if (!checkAnswer(i)) {
      clientLives[playerNums[i]] = clientLives[playerNums[i]] - 1; // Remove a life
      clientOutputs[playerNums[i]] = 4; //  Send "FAIL"
      if (clientLives[playerNums[i]] > 0) stillAlive = 1; // If still alive, set flag
    } else {
      clientOutputs[playerNums[i]] = 3; //  Send "PASS"
    }
  }
  sleep(0.5); //  Allow sending of outputs...
  if (stillAlive) { //  If a player is still alive
    for (int i = 0; i < numPlayers; i++) {
      if (clientLives[playerNums[i]] == 0) {  //  If they were eliminated in the last round
        clientLives[playerNums[i]] = -1;  // Set them to "ELIM" life state
        clientOutputs[playerNums[i]] = 5;
      }
    }
    return 1; //  Game still playing
  } else {
    for (int i = 0; i < numPlayers; i++) {
      if (clientLives[playerNums[i]] == 0) {  //  If they were eliminated in the last round
        clientOutputs[playerNums[i]] = 6; // Send a victory cheer
      }
    }
    return 0; //  Game finished
  }
}

//  REMOVE ALL PLAYERS
//  Removes all the players from the game
int removeAllPlayers() {
  memset(clientLives, 0, MAX_CONNECTIONS);
  memset(playerNums, 0, MAX_PLAYERS);
  numPlayers = 0;
}

//  MAIN
//  Executes the application when all functions have been loaded into memory 
int main() {start();}