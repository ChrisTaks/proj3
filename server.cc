// Copyright 2024 Chris Taks

#include <proj3/server.h>
#include <proj3/shm.h>

#include <iostream>
#include <sys/sysinfo.h>

void DomainSocketServer::Run() {
  // Domain Socket stuff.
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

  // cleanup.
  signal(SIGTERM, exit);
  signal(SIGINT, exit);

  // make sure semaphores do not already exit.
  sem_unlink(SERVER_SEM);
  sem_unlink(CLIENT_SEM);

  // create new semaphores.
  sem_t *semServer = sem_open(kServerSem, O_CREAT, 0660, 0);
  sem_t *semClient = sem_open(kClientSem, O_CREAT, 0660, 0);

  // STEP 1
  std::cout << "SERVER STARTED" << std::endl;

  while (true) {
    std::vector<std::string> badArgs;
    if (!Accept(&socket_fd)) {
      std::cout << "CLIENT NOT ACCEPTED" << std::endl;
    }
    if (socket_fd < 0) {
      std::cerr << "Socket connection: " << ::strerror(errno) << std::endl;
      continue;
    }

    // STEP 2
    // reading the message sent from the client.
    std::string msg;
     ::ssize_t bytes_read = Read(&msg, socket_fd);

     if (bytes_read < 0) {
       std::cerr << "Server shutting down..." << std::endl;
       exit(0);
      }

    std:: cout << "CLIENT REQUEST RECEIVED" << std::endl;

    // wait for client to open shared memory.
    sem_wait(semClient);  // 1C (wait on 1C to post).

    // STEP 3
    // open the shared memory.
    int shmfd = shm_open(SHMPATH, O_RDWR, 0);
    store_ = reinterpret_cast<struct SharedMemoryStore*>(mmap(NULL,
                kSharedMemSize,
                PROT_READ | PROT_WRITE,
                MAP_SHARED,
                shmfd,
                0));

    if (store_ == MAP_FAILED) {
      std::cout << "MAP_FAILED" << std::endl;
    }
    std::cout << "\tMEMORY OPEN" << std::endl;

    // wait for client to be ready to read.
    sem_wait(semClient);  // 2C (wait on 2C to post).

    // parse the msg and add to theArgs whenever a delimiter is found.
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

    // initializing the string vectors that will be written to memory.
    std::string oneFourth;
    std::string twoFourths;
    std::string threeFourths;
    std::string fourFourths;

    // STEP 4
    // opening file.
    std::cout << "\tOPENING: \"" << theArgs[0] << "\"" <<std::endl;
    std::ifstream equationFile(theArgs[0]);
    if (!equationFile.is_open()) {
      sem_post(semServer);  // 1S (if file fails).
      std::cerr << "\tFILE FAILED TO OPEN" << std::endl;
      ::ssize_t bytes_wrote = Write("1", socket_fd);
      if (bytes_wrote < 0) {
        std::cerr << "\tFAILED TO WRITE TO SOCKET" << std::endl;
      }
      Close(socket_fd);
    } else {
      // file succeeds
      ::size_t bytes_wrote = Write("0", socket_fd);
      if (bytes_wrote < 0) {
        std::cerr << "\tFAILED TO WRITE TO SOCKET" << std::endl;
      }
      // identify the amount of lines.
      int lineCount = std::stoi(theArgs[1]);

      // dividing the file up.
      int counter = 0;
      findLineNumberDivisibleBy4(&lineCount, &counter);

      // File reading.
      std::string equationLine;
      int lineNumber = 0;
      while (std::getline(equationFile, equationLine)) {
        ++lineNumber;
        if (lineNumber <= lineCount * 1 / 4) {
          // add to first vector.
          oneFourth += equationLine += kUS;
        } else if (lineNumber <= lineCount * 2 / 4) {
          // add to second vector.
          twoFourths += equationLine += kUS;
        } else if (lineNumber <= lineCount * 3 / 4) {
          // add to third vector.
          threeFourths += equationLine += kUS;
        } else {
          // add to fourth vector (will also add the lines not in lineCount).
          fourFourths += equationLine += kUS;
        }
      }
      equationFile.close();
      std::cout << "\tFILE CLOSED" << std::endl;

      // load the file into shared memory.
      strncpy(store_->buffer[0], oneFourth.c_str(), kMemFourthSize);
      strncpy(store_->buffer[1], twoFourths.c_str(), kMemFourthSize);
      strncpy(store_->buffer[2], threeFourths.c_str(), kMemFourthSize);
      strncpy(store_->buffer[3], fourFourths.c_str(), kMemFourthSize);

      sem_post(semServer);  // 1S (posting, client can now read).
    }
    // TODO: close shared memory.
    ::munmap(store_, sizeof(store_));
    ::close(shmfd);
    std::clog << "\tMEMORY CLOSED" << std::endl;
  }  // end while main while loop.
  
  // destroy semaphores.
  sem_unlink(SERVER_SEM);
  sem_unlink(CLIENT_SEM);
}

void findLineNumberDivisibleBy4(int *lineNumber, int *counter) {
  if (*lineNumber % 4 != 0) {
    while (*lineNumber % 4 != 0) {
      --*lineNumber;
      ++*counter;
    }
  }
}

int main(int arc, char *argv[]) {
  DomainSocketServer dss(kSocket_path);
  dss.Run();
  return 0;
}
