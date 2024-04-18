# CSCE 311: Project 3: Shared Memory and Threading
***
## Chris Taks
***
This project consists of two programs, a client and a server. The client will take the arguments: filename, and the number of lines in the file. The files that you can choose from are in ./proj3/dat. Every line in the files consists of a simple (albeit long) math equation.
The client will connect to the server over a Unix Domain Socket to give it the file path along with the number of lines.
The client will also open shared memory with the server.
The server will read in the client's arguments and then open the file to read all the equations in that file.
Once the server reads the equations, it will write them to the shared memory.
The client will then read from the shared memory and proceed to process every equation that was written. It evenly distributes every equation to 4 threads (created using POSIX threads).
The threads then calculate every equation and sum them all together.
Important information is printed out to STDOUT.

The client and server run off their respective .cc files, with supporting .h files.

There is a supporting domain_socket.cc and domain_socket.h file that handles the Unix Domain Socket.

There is a supported shm.h file that handles the shared memory data. "shm" is short for "shared memory".

A makefile is also included to compile the source files. `make server` complies the server. `make client` complies the client. `make all` compiles both at the same time.
