// Copyright 2024 Chris Taks

#ifndef _PROJ3_SERVER_H_
#define _PROJ3_SERVER_H_

#include <proj3/domain_socket.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <cstring>

#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <iostream>

const char kSocket_path[] = "socket_example";

class DomainSocketServer : public DomainSocket {
 public:
    using ::DomainSocket::DomainSocket;

    void Run();
};

#endif  // _SERVER_H_
