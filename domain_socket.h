// Copyright 2024 Chris Taks

#ifndef _PROJ3_DOMAIN_SOCKET_H_
#define _PROJ3_DOMAIN_SOCKET_H_

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/unistd.h>

#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <cstring>

#include <string>
#include <vector>
#include <iostream>

class DomainSocket {
 public:
    static const char kEoT = '\004';

    static const char kUS = '\037';

    explicit DomainSocket(const char *socket_path, bool abstract = true);

    bool Init(int domain = AF_UNIX, int type = SOCK_STREAM, int protocol = 0);

    bool Bind();

    bool Listen(std::size_t max_connections = 1) const;

    bool Accept(int* client_request_socket_filedesc) const;

    bool Connect() const;

    ::ssize_t Read(std::string* buffer,
                   int socket_file_descriptor = 0,
                   std::size_t return_after_bytes = 0,
                   char end_of_transmission = DomainSocket::kEoT) const;

    ::ssize_t Write(const std::string& bytes,
                    int socket_file_desciptor = 0,
                    char end_of_transmission = DomainSocket::kEoT) const;

    void Close(int socket_file_descriptor = 0) const;

 protected:
    int socket_fd_;
    ::sockaddr_un sock_addr_;

    std::string socket_path_;

 private:
    ssize_t Read(int socket_fd, char buffer[], std::size_t buffer_size) const;
};

#endif  // _PROJ2_DOMAIN_SOCKET_H_
