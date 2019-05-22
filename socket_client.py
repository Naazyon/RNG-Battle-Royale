"""
This is a simple example of a client program written in Python.
Again, this is a very basic example to complement the 'basic_server.c' example.


When testing, start by initiating a connection with the server by sending the "init" message outlined in 
the specification document. Then, wait for the server to send you a message saying the game has begun. 

Once this message has been read, plan out a couple of turns on paper and hard-code these messages to
and from the server (i.e. play a few rounds of the 'dice game' where you know what the right and wrong 
dice rolls are). You will be able to edit this trivially later on; it is often easier to debug the code
if you know exactly what your expected values are. 

From this, you should be able to bootstrap message-parsing to and from the server whilst making it easy to debug.
Then, start to add functions in the server code that actually 'run' the game in the background. 
"""

import socket
import threading
from time import sleep
# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Connect the socket to the port where the server is listening
server_address = ('localhost', 4000)
print ('connecting to %s port %s' % server_address)
sock.connect(server_address)

state = "RECEIVING"

def listenSocket():
  while(state != "SENDING"):
    data = sock.recv(1024)
    if (data):
      mess = data.decode()
      print(mess)
      

listenThread = threading.Thread(target=listenSocket)
listenThread.daemon = True
listenThread.start()

while True:
  message = input("").encode()
  if (message):
    state = "SENDING"
    sleep(1/100)
    sock.sendall(message)
    state = "RECEIVING"