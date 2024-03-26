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

const char kSocket_path[] = "ctaks_socket";

class DomainSocketClient : public DomainSocket {
 public:
    using DomainSocket::DomainSocket;

    void Run(int argc, char *argv[]);

    double AddNumbers(double a, double b);

    double SubtractNumbers(double a, double b);

    double MultiplyNumbers(double a, double b);

    double DivideNumbers(double a, double b);

    bool IsOperator(std::string arg);

    std::string processEquation(std::string line);

 private:
    static const std::size_t kBufferSize = 1024;
    static const std::size_t kSharedMemSize =
    sizeof(SharedMemoryStore<kBufferSize>);

    SharedMemoryStore<kSharedMemSize> *store_;
};

#endif  // _PROJ3_CLIENT_H_
