// Copyright 2024 Chris Taks

#include <proj3/server.h>
#include <proj3/shm.h>

#include <iostream>
#include <sys/sysinfo.h>

// struct SharedMemoryStore* shmpt;

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
  signal(SIGTERM, exit);
  signal(SIGINT, exit);

  // make sure semaphores do not already exit
  sem_unlink(SERVER_SEM);
  sem_unlink(CLIENT_SEM);

  // create new semaphores
  sem_t *semServer = sem_open(SERVER_SEM, O_CREAT, 0660, 0);
  sem_t *semClient = sem_open(CLIENT_SEM, O_CREAT, 0660, 0);
  int semVal1;
  int semVal2;
  sem_getvalue(semServer, &semVal1);
  std::cout << "[WIDPIO]: semServer created. Val: " << semVal1 << std::endl;
  sem_getvalue(semClient, &semVal2);
  std::cout << "[WIDPIO]: semClient created. Val: " << semVal2 << std::endl;

  // STEP 1
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

    // STEP 2
    // reading the message sent from the client
    std::string msg;
     ::ssize_t bytes_read = Read(&msg, socket_fd);

     if (bytes_read < 0) {
       std::cerr << "Server shutting down..." << std::endl;
       exit(0);
      }
  
    std:: cout << "CLIENT REQUEST RECEIVED" << std::endl;

    // TODO: open shared memory
    // struct SharedMemoryStore shmp;

    // proper cleanup
    

    // wait for client to open shared memory
    std::cout << "[WIDPIO]: waiting on client to open shm" << std::endl;
    int semVal;
    sem_getvalue(semClient, &semVal);
    std::cout << "[WIDPIO]: semClient: " << semVal << std::endl;
    sem_wait(semClient); // 1C (wait on 1C to post)

    // so does shm_open
    int shmfd = shm_open(SHMPATH, O_RDWR, 0);

    std::cout << "[WIDPIO]: linking shared memory" << std::endl;
    store_ = static_cast<SharedMemoryStore<kSharedMemSize>*>(mmap(NULL,
                kSharedMemSize,
                PROT_READ | PROT_WRITE,
                MAP_SHARED,
                shmfd,
                0));
    
    //char read_buffer[kBufferSize];

    // wait for client to be ready to read
    std::cout << "[WIDPIO]: waiting on client to be ready to read" << std::endl;
    sem_wait(semClient); // 2C (wait on 2C to post)

    // sem_post(semServer); // 0S (letting client post 2C) this deadlocks btw

    // parse the msg and add to theArgs whenever a delimiter is found
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
    std::cout << "\tOPENING: \"" << theArgs[0] << "\"" <<std::endl;
    std::ifstream equationFile(theArgs[0]);
    if (!equationFile.is_open()) {
      errorPath += theArgs[0];
      error_bytes = Write(errorPath, socket_fd);
      std::cout << "BYTES SENT: " << error_bytes << std::endl;
    } else {
      // identify the amount of lines
      int lineNumbers = std::stoi(theArgs[1]);

      int lineNumber = 0;
      std::string equationLine;
      std::string errorLine = "FILE HAS DIFFERING NUMBER OF LINES THAN ARGUMENT ";

      while (std::getline(equationFile, equationLine)) {
        ++lineNumber;
        finalLine.push_back(equationLine);
      }
    bool foundBadLine = false;
    if (lineNumber != lineNumbers) {
      foundBadLine = true;
    }
    if (foundBadLine) {
      error_bytes = Write(errorLine, socket_fd);
      std::cout << "BYTES SENT: " << error_bytes << std::endl;
    }
    }
    equationFile.close();

    // builds a single string from the vector of final lines
    std::string finalInput;
    for (std::string line : finalLine) {
      finalInput += line;
      finalInput += kUS;
    }
    // finalInput += kEoT;

    std::cout << "[WIDPIO]: finalInput: " << finalInput << std::endl;
    // char write_buffer[kBufferSize] = "just a test\n";
    const char *write_buffer = finalInput.c_str();

    // while(semClient == 0) {
    // // load finalInput into shared memory
    // snprintf(shmpt->buffer, shmp.kBuffSize, "%s", finalInput);
    // }

    // load finalInput into shared memory
    // TODO: make what is loaded into shared memory be a character array.
    // TODO: learn about the difference between character arrays and strings, etc
    // TODO: learn about writing to shared memory. look at lewis code, etc
    snprintf(store_->buffer, kSharedMemSize, "%s", write_buffer);

    sem_post(semServer); // 1S (posting, client can now read)


  }
  //   // send the data
  //   ::size_t bytes_wrote;
  //   if (!error_bytes) {
  //     bytes_wrote = Write(finalInput, socket_fd);
  //     std::cout << "BYTES SENT: " << bytes_wrote << std::endl;
  //   }
  //     if (bytes_wrote < 0) {
  //       std::cerr << "Server terminating..." << std::endl;
  //       exit(3);
  //     }
     Close(socket_fd);
  //   // clears
  //   finalLine.clear();
  //   theArgs.clear();
  // }  // end while loop
}

int main(int arc, char *argv[]) {
  DomainSocketServer dss(kSocket_path);
  dss.Run();
  return 0;
}
