// Copyright 2024 Chris Taks

#include <proj3/server.h>

#include <iostream>
#include <sys/sysinfo.h>

void DomainSocketServer::Run() {
  int socket_fd;

  if (!Init()) {
    exit(-1);
  }

  if (!Bind()) {
    exit(-2);
  }

  size_t sizenum = 2;
  if (!Listen(sizenum)) {
    exit(-3);
  }
  std::cout << "SERVER STARTED" << std::endl;
  std::cout << "MAX CLIENTS: " << get_nprocs_conf() << std::endl;

  while (true) {
    std::vector<std::string> badArgs;
    if (!Accept(&socket_fd)) {
      std::cout << "CLIENT NOT ACCEPTED" << std::endl;
    }
    if (socket_fd < 0) {
      std::cerr << "Socket connection: " << ::strerror(errno) << std::endl;
      continue;
    }
    std:: cout << "CLIENT CONNECTED" << std::endl;

    std::string msg;
     ::ssize_t bytes_read = Read(&msg, socket_fd);
     if (bytes_read < 0) {
       std::cerr << "Server shutting down..." << std::endl;
       exit(0);
      }
    std::vector<std::string> theArgs;
    std::string toBeAdded;
    for (size_t i = 0; i < msg.size(); ++i) {
      if ((msg[i] == kUS || msg[i] == kEoT) && toBeAdded.size() > 0) {
        theArgs.push_back(toBeAdded);
        toBeAdded = "";
      } else {
        toBeAdded += msg[i];
      }
    }
    std::vector<std::string> finalLine;
    std::string errorPath = "INVALID FILE: ";
    std::size_t error_bytes = 0;
    std::cout << "PATH: \"" << theArgs[0] << "\"" <<std::endl;
    std::ifstream equationFile(theArgs[0]);
    if (!equationFile.is_open()) {
      errorPath += theArgs[0];
      error_bytes = Write(errorPath, socket_fd);
      std::cout << "BYTES SENT: " << error_bytes << std::endl;
    } else {
    // sort line numbers
    std::vector<int> lineNumbers;
    for (size_t i = 1; i < theArgs.size(); ++i) {
      lineNumbers.push_back(std::stoi(theArgs[i]));
    }
    std::sort(lineNumbers.begin(), lineNumbers.end());
    std::cout << "LINES: ";
    for (size_t i = 0; i < lineNumbers.size(); ++i) {
      std::cout << lineNumbers[i];
      if (i < lineNumbers.size()-1) {
        std::cout << ", ";
      }
    }
    std::cout << std::endl;

    int lineNumber = 0;
    std::string equationLine;
    std::string errorLine = "INVALID LINE NO ";
    while (std::getline(equationFile, equationLine)) {
      ++lineNumber;
      for (int line : lineNumbers) {
        if (line == lineNumber) {
          finalLine.push_back(equationLine);
        }
      }
    }
    bool foundBadLine = false;
    int badLine;
    for (int line : lineNumbers) {
      if (line > lineNumber || line < 1) {
        foundBadLine = true;
        badLine = line;
      }
    }
    if (foundBadLine) {
      errorLine += std::to_string(badLine);
      error_bytes = Write(errorLine, socket_fd);
      std::cout << "BYTES SENT: " << error_bytes << std::endl;
    }
    }
    equationFile.close();

    std::string finalInput;
    for (std::string line : finalLine) {
      finalInput += line;
      finalInput += kUS;
    }
    finalInput += kEoT;
    // send the data
    ::size_t bytes_wrote;
    if (!error_bytes) {
      bytes_wrote = Write(finalInput, socket_fd);
      std::cout << "BYTES SENT: " << bytes_wrote << std::endl;
    }
      if (bytes_wrote < 0) {
        std::cerr << "Server terminating..." << std::endl;
        exit(3);
      }
    Close(socket_fd);
    // clears
    finalLine.clear();
    theArgs.clear();
  }  // end while loop
}

int main(int arc, char *argv[]) {
  DomainSocketServer dss(argv[1]);
  dss.Run();
  return 0;
}
