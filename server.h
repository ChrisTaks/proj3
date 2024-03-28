// Copyright 2024 Chris Taks

#ifndef _PROJ3_SERVER_H_
#define _PROJ3_SERVER_H_

#include <proj3/domain_socket.h>
#include <proj3/shm.h>

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

const char kSocket_path[] = "ctaks_socket";

class DomainSocketServer : public DomainSocket {
 public:
    using ::DomainSocket::DomainSocket;

    void Run();

 private:
     SharedMemoryStore *store_;
     static const std::size_t kSharedMemSize = sizeof(store_->buffer);
};

void findLineNumberDivisibleBy4(int *lineNumber, int *counter);

#endif  // _SERVER_H_
