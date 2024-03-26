# Copyright 2024 Chris Taks

# uses g++ compiler
cc = g++

# path for include directive and linger
path = ..

# compile with C++ 20 standard
standard = -std=c++2a

flags += -Wall
flags += -pedantic
flags += -I $(path)
flags += -g

linker_flags += -pthread
# linker_flags += -lrt

# link = $(cc) $(flags) -o
link = $(cc) $(linker_flags) -o

compile = $(cc) $(flags) -c -o

all: client server

client : client.o domain_socket.o
	$(link) $@ $^ -lrt

client.o : client.cc client.h shm.h
	$(compile) $@ $<

server: server.o domain_socket.o
	$(link) $@ $^ -lrt

server.o : server.cc server.h shm.h
	$(compile) $@ $<

domain_socket.o: domain_socket.cc domain_socket.h shm.h
	$(compile) $@ $<

clean: 
	$(RM) *.o $(all)
