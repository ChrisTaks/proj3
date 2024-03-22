// Copyright 2024 Chris Taks

#ifndef _PROJ3_CLIENT_H_
#define _PROJ3_CLIENT_H_

#include <proj3/domain_socket.h>

#include <cstddef>
#include <cstdlib>

#include <string>
#include <algorithm>
#include <iostream>

const char kSocket_path[] = "socket_example";

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
};

#endif  // _PROJ2_CLIENT_H_
