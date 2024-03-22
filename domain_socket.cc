// Copyright 2024 Chris Taks

#include <proj3/domain_socket.h>

DomainSocket::DomainSocket(const char *socket_path, bool abstract) {
  socket_fd_ = 0;

  socket_path_ = std::string(socket_path);

  sock_addr_ = {};
  sock_addr_.sun_family = AF_UNIX;

  if (abstract) {
    strncpy(sock_addr_.sun_path + 1,
            socket_path,
            sizeof(sock_addr_.sun_path) -1);
  } else {
    strncpy(sock_addr_.sun_path,
            socket_path,
            sizeof(sock_addr_.sun_path));
  }
}

bool DomainSocket::Init(int domain, int type, int protocol) {
  socket_fd_ = ::socket(domain, type, protocol);
  if (socket_fd_ > 0) {
    return true;
  }
  std::cerr << "DomainSocket::Init, " << strerror(errno) << std::endl;
  return false;
}

bool DomainSocket::Bind() {
  unlink(socket_path_.c_str());

  if (!bind(socket_fd_,
      reinterpret_cast<const sockaddr*>(&sock_addr_),
      sizeof(sock_addr_))) {
    return true;
  }
  std::cerr << "DomainSocket::Bind(), " << strerror(errno) << std::endl;
  return false;
}

bool DomainSocket::Listen(std::size_t max_connections) const {
  if (!listen(socket_fd_, max_connections)) {
    return true;
  }
  std::cerr << "DomainSocket::Listen, " << strerror(errno) << std::endl;
  return false;
}

bool DomainSocket::Accept(int* req_socket_fd) const {
  *req_socket_fd = accept(socket_fd_, nullptr, nullptr);
  if (req_socket_fd) {
    return true;
  }
  std::cerr << "DomainSocket::Accept(), " << strerror(errno) << std::endl;
  return false;
}

bool DomainSocket::Connect() const {
  if (!connect(socket_fd_,
      reinterpret_cast<const sockaddr*>(&sock_addr_),
      sizeof(sock_addr_))) {
    return true;
  }

  std::cerr << "DomainSocket::Connect(), " << strerror(errno) << std::endl;
  return false;
}

::ssize_t DomainSocket::Read(std::string* output,
                             int socket_fd,
                             std::size_t byte_count,
                             char eot) const {
  if (!socket_fd) {
    socket_fd = socket_fd_;
  }
  const size_t kBufferSize = 64;
  char buffer[kBufferSize];

  ::ssize_t bytes_read = Read(socket_fd,
                              buffer,
                              byte_count ? byte_count : kBufferSize);

  if (bytes_read <= 0) {
    return bytes_read;
  }
  ::ssize_t total_bytes_read = bytes_read;

  while (buffer[bytes_read -1] != eot
         || (byte_count && static_cast<std::size_t>(bytes_read) < byte_count)) {
    output->insert(output->size(), buffer, bytes_read);
    bytes_read = Read(socket_fd, buffer,
                      byte_count ? byte_count - total_bytes_read : kBufferSize);

    if (bytes_read <= 0) {
      return total_bytes_read;
    }
    total_bytes_read += bytes_read;
  }
  output->insert(output->size(),
                 buffer,
                 byte_count ? bytes_read : bytes_read - 1);

  return total_bytes_read;
}

::ssize_t DomainSocket::Read(int socket_fd,
                             char buffer[],
                             std::size_t buffer_size) const {
  ::ssize_t bytes_read = ::read(socket_fd, buffer, buffer_size);

  if (bytes_read == 0) {
    // connection terminated by writer
    std::cout << "Writer disconnected" << std::endl;
  } else if (bytes_read < 0) {
    std::cerr << "Read Error: " << ::strerror(errno) << std::endl;
  }

  return bytes_read;
}

::ssize_t DomainSocket::Write(const std::string& bytes,
                              int socket_fd,
                              char eot) const {
  if (!socket_fd)
    socket_fd = socket_fd_;

  ::ssize_t bytes_wrote, total_bytes_wrote = 0;

  if (socket_fd_ <= 0) {
    std::cerr << "DomainSocket::Write, Socket file descriptor not initialized."
      << std::endl;
    return -1;
  }

  do {
    bytes_wrote = ::write(socket_fd,
                          bytes.c_str() + total_bytes_wrote,
                          bytes.size());  // send cstring null term, i.e. \0
    if (bytes_wrote < 0) {
      std::cerr << "Write Error: " << ::strerror(errno) << std::endl;

      return bytes_wrote;
    }
    total_bytes_wrote += bytes_wrote;  // accumulate bytes written
  } while (total_bytes_wrote < static_cast<::ssize_t>(bytes.size()));
  // send end of transmission character
  const char kTerminateMessage[] = { eot };
  bytes_wrote = write(socket_fd, kTerminateMessage, sizeof(kTerminateMessage));
  if (bytes_wrote < 0) {
      std::cerr << "Write Error: " << ::strerror(errno) << std::endl;

      return bytes_wrote;
  }
  return total_bytes_wrote + 1;  // + eot char
}

void DomainSocket::Close(int socket_file_descriptor) const {
  if (socket_file_descriptor)
    ::close(socket_file_descriptor);
  else
    ::close(socket_fd_);
}
