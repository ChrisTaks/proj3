// Copyright 2024 Chris Taks

#ifndef _PROJ3_CLIENT_H_
#define _PROJ3_CLIENT_H_

#include <proj3/domain_socket.h>
#include <proj3/shm.h>

#include <sys/mman.h>
#include <sys/unistd.h>

#include <cstddef>
#include <cstdlib>

#include <string>
#include <algorithm>
#include <iostream>
#include <pthread.h>

const char kSocket_path[] = "ctaks_socket";

const int kThreadNum = 4;

SharedMemoryStore *store_;
static const std::size_t kSharedMemSize = sizeof(store_->buffer);

class DomainSocketClient : public DomainSocket {
 public:
    using DomainSocket::DomainSocket;

    void Run(int argc, char *argv[]);
};

struct ThreadData {
  int memArrayNumber;
  int operations = 0;
  double sum = 0;
};

void* processThread(void* input);

double AddNumbers(double a, double b);

double SubtractNumbers(double a, double b);

double MultiplyNumbers(double a, double b);

double DivideNumbers(double a, double b);

bool IsOperator(std::string arg);

double processEquation(std::string line);

#endif  // _PROJ3_CLIENT_H_
