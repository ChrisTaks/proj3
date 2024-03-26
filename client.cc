// Copyright 2024 Chris Taks

#include <proj3/client.h>
#include <proj3/shm.h>

#include <semaphore.h>

#include <iostream>
#include <vector>
#include <string>

// struct SharedMemoryStore<kBufferSize> *shmpt;

void DomainSocketClient::Run(int argc, char *argv[]) {
  std::cout << "Client initializing..." << std::endl;
  if (!Init()) {
    exit(1);
  }

  std::cout << "Client connecting..." << std::endl;
  if (!Connect()) {
    exit(2);
  }
  std::cout << "SERVER CONNECTION ACCEPTED" << std::endl;

  // send the data
  std::string finalInput;
  for (int i = 1; i < argc; ++i) {
    std::string input(argv[i]);
    finalInput += input;
    finalInput += kUS;
  }
  finalInput += kEoT;
  std::cout << "argc: " << argc << std::endl;
  std::cout << "[WIDPIO]: to be written: \"" << finalInput << "\"" << std::endl;
  
  // write the input
  ::size_t bytes_wrote = Write(finalInput);
  std::cout << "[WIDPIO]: done writing" << std::endl;

  if (bytes_wrote < 0) {
    std::cerr << "Client terminating..." << std::endl;
    exit(3);
  } else if (bytes_wrote == 0) {
    std::cerr << "Server disconnected" << std::endl;
    exit(4);
  }

  //struct SharedMemoryStore shmp;
  // make sure shared memory does not already exist
  shm_unlink(SHMPATH);

  // open existing sempaphores on server
  std::cout << "[WIDPIO]: connecting semaphores" << std::endl;
  sem_t *semServer = sem_open(SERVER_SEM, 0);
  sem_t *semClient = sem_open(CLIENT_SEM, 0);

  int semVal1;
  int semVal2;
  sem_getvalue(semServer, &semVal1);
  std::cout << "[WIDPIO]: semServer connected. Val: " << semVal1 << std::endl;
  sem_getvalue(semClient, &semVal2);
  std::cout << "[WIDPIO]: semClient connected. Val: " << semVal2 << std::endl;


  // create shared memory  
  int shmfd = shm_open(SHMPATH, O_CREAT | O_EXCL | O_RDWR,
                         S_IRUSR | S_IWUSR );

  // Truncate the memory
  if (::ftruncate(shmfd, kSharedMemSize) < 0) {
    std::cerr << ::strerror(errno) << std::endl;
    ::exit(errno);
  }
  
  // map the shared memory
  store_  = static_cast<SharedMemoryStore<kSharedMemSize>*>(
              ::mmap(NULL,
              kSharedMemSize,
              PROT_READ | PROT_WRITE,
              MAP_SHARED,
              shmfd,
              0));

  if (store_ == MAP_FAILED) {
    std::cerr << strerror(errno) << std::endl;
    exit(errno);
  }

  while (semServer == 0) {}

    std::cout << "[WIDPIO]: entering semaphore while loop" << std::endl;
    // notify server that shared memory is created
    std::cout << "[WIDPIO]: notifying server that shm is created" << std::endl;
    int semVal3;
    sem_getvalue(semClient, &semVal3);
    std::cout << "[WIDPIO]: semClient before val: " << semVal3 << std::endl;
    sem_post(semClient); // 1C (posting, letting 1C go on server side)
    int semVal;
    sem_getvalue(semClient, &semVal);
    std::cout << "[WIDPIO]: semClient after val: " << semVal << std::endl;

    // notify server that client is ready to read
    // sem_wait(semServer); // 0S (waiting on server to wait on client) this deadlocks btw

    sem_post(semClient); // 2C (posting, letting 2C go on server side)
    sem_getvalue(semClient, &semVal);
    std::cout << "[WIDPIO]: semClient after val: " << semVal << std::endl;

  // wait on server to be finished writing to sharedmem
  std::cout << "[WIDPIO]: waiting on server to finish writing" << std::endl;
    int semVal4;
    sem_getvalue(semClient, &semVal4);
    std::cout << "[WIDPIO]: semClient: " << semVal4 << std::endl;
  sem_wait(semServer); // 1S (waiting on 1S server to post)

  // read string from shared memory
  // char read_buffer[kBufferSize];
  std::string message;
  std::cout << "[WIDPIO]: reading from shm" << std::endl;
  // snprintf(read_buffer, kBufferSize, "%s", store_->buffer);
  //snprintf(read_buffer, kBufferSize, "%s", store_->strBuffer);

  std::cout << "[WIDPIO]: checking if read: " << store_->buffer << std::endl;
  // for (int i = 0; i < sizeof(store_->buffer); ++i) {
  //   if (store_->buffer[i] == NULL) {
  //     // std::cout << i << " ";
  //   } else {
  //     std::cout << store_->buffer[i];
  //   }
  // }
  // std::cout << std::endl;

  // // TODO: fix below this
  // // recieve the data
  // std::string msg;

  // ::ssize_t bytes_read = Read(&msg);
  // std::cout << "BYTES RECEIVED: " << bytes_read << std::endl;
  // if (bytes_read < 0) {
  //   std::cerr << "Server shutting down..." << std::endl;
  //   exit(0);
  // } else if (bytes_read) {
  //   Close(socket_fd_);
  // }

  // ::size_t found = msg.find("INVALID");
  // if (found != std::string::npos) {
  //   std::cout << "ERROR: " << msg << std::endl;
  //   exit(1);
  // }

  // // print/process the data
  // std::vector<std::string> theLines;
  // std::string toBeAdded;
  // for (size_t i = 0; i < msg.size(); ++i) {
  //   if ((msg[i] == kUS || msg[i] == kEoT) && toBeAdded.size() > 0) {
  //     theLines.push_back(toBeAdded);
  //     toBeAdded = "";
  //   } else {
  //     toBeAdded += msg[i];
  //   }
  // }

  // // sort line numbers
  // std::vector<int> lineNumbers;
  // for (int i = 3; i < argc; ++i) {
  //   lineNumbers.push_back(std::stoi(argv[i]));
  // }
  // std::sort(lineNumbers.begin(), lineNumbers.end());

  // int lineNumber = 0;
  // for (std::string line : theLines) {
  //   std::string finishedLine = "line ";
  //   finishedLine += std::to_string(lineNumbers[lineNumber]);
  //   finishedLine += ": ";
  //   finishedLine += line;
  //   finishedLine += " = ";
  //   finishedLine += processEquation(line);
  //   std::cout << finishedLine << std::endl;
  //   ++lineNumber;
  // }
}

double DomainSocketClient::AddNumbers(double a, double b) {
  return a + b;
}

double DomainSocketClient::SubtractNumbers(double a, double b) {
  return a - b;
}

double DomainSocketClient::MultiplyNumbers(double a, double b) {
  return a * b;
}

double DomainSocketClient::DivideNumbers(double a, double b) {
  return a / b;
}

bool DomainSocketClient::IsOperator(std::string arg) {
  return arg == "+" || arg == "-" || arg == "x" || arg == "/";
}

std::string DomainSocketClient::processEquation(std::string line) {
  std::vector<double> numbers;
  std::vector<std::string> operators;
  // break up the string
  std::vector<std::string> args;
  std::string component;
  for (char c : line) {
    if (c != ' ') {
      component += c;
    } else {
      args.push_back(component);
      component = "";
    }
  }
  args.push_back(component);
  // adding the args to their respective vectors
  for (size_t i = 0; i < args.size(); ++i) {
    if (IsOperator(args[i])) {
      operators.push_back(args[i]);
      if (operators.back() == "x" || operators.back() == "/") {
        double a = numbers.back();
        numbers.pop_back();
        if (operators.back() == "x") {
          numbers.push_back(MultiplyNumbers(a, std::stod(args[i+1])));
        }
        if (operators.back() == "/") {
          numbers.push_back(DivideNumbers(a, std::stod(args[i+1])));
        }
        ++i;  // iterate past the next variable since it was already used
      }
    } else {
      numbers.push_back(std::stod(args[i]));
    }
  }
  // next, just add / subtract the remaining numbers
  for (std::string& op : operators) {
    if (op == "+") {
      numbers[0] = AddNumbers(numbers[0], numbers[1]);
      numbers.erase(numbers.begin()+1);
    }
    if (op == "-") {
      numbers[0] = SubtractNumbers(numbers[0], numbers[1]);
      numbers.erase(numbers.begin()+1);
    }
  }
  return std::to_string(static_cast<int>(numbers[0]));
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Wrong amount of arguments. Client terminating...\n"
              << "./client ./dat/equations_<number>.txt <number>" 
              << std::endl;
    return 0;
  }

  DomainSocketClient dsc(kSocket_path);
  dsc.Run(argc, argv);

  return 0;
}
