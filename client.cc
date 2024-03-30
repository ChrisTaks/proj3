// Copyright 2024 Chris Taks

#include <proj3/client.h>
#include <proj3/shm.h>

#include <semaphore.h>

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

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



  // make sure shared memory does not already exist
  shm_unlink(SHMPATH);

  // open existing sempaphores on server
  sem_t *semServer = sem_open(kServerSem, 0);
  sem_t *semClient = sem_open(kClientSem, 0);

  // create shared memory.
  int shmfd = shm_open(SHMPATH, O_CREAT | O_EXCL | O_RDWR,
                         S_IRUSR | S_IWUSR);

  // Truncate the memory
  if (::ftruncate(shmfd, kSharedMemSize) < 0) {
    std::cerr << ::strerror(errno) << std::endl;
    ::exit(errno);
  }

  // map the shared memory
  store_  = reinterpret_cast<struct SharedMemoryStore*>(
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

  std::cout << "SHARED MEMORY ALLOCATED: " << kSharedMemSize
            << " BYTES" << std::endl;

  // write the input
  ::size_t bytes_wrote = Write(finalInput);

  if (bytes_wrote < 0) {
    std::cerr << "Client terminating..." << std::endl;
    exit(3);
  } else if (bytes_wrote == 0) {
    std::cerr << "Server disconnected" << std::endl;
    exit(4);
  }
  while (semServer == 0) {}

    // notify server that shared memory is created
    sem_post(semClient);  // 1C (posting, letting 1C go on server side)

    // notify server that client is ready to read
    sem_post(semClient);  // 2C (posting, letting 2C go on server side)

  // wait on server to be finished writing to sharedmem
  sem_wait(semServer);  // 1S (waiting on 1S server to post)

  std::string serverMessage;
  ::ssize_t bytes_read = Read(&serverMessage);
  if (bytes_read < 0) {
    std::cerr << "BYTES FAILED TO WRITE" << std::endl;
  }

  if (std::stoi(serverMessage)) {
    std::cout << "ERROR: INVALID FILE \"" << argv[1] << "\"" << std::endl;
    exit(1);
  }

  std::locale comma_locale("en_US.UTF-8");
  std::cout << std::fixed << std::setprecision(2);
  std::cout.imbue(comma_locale);

  // STEP 3
  // creating structs of pthread data
  struct ThreadData threadData[kThreadNum];
  for (int i = 0; i < kThreadNum; ++i) {
    threadData[i].memArrayNumber = i;
  }

  // creating pthreads
  pthread_t threads[kThreadNum];
  for (pthread_t t_id = 0; t_id < 4; ++t_id) {
    ::pthread_create(&threads[t_id],
                     NULL,
                     processThread,
                     reinterpret_cast<void*>(&threadData[t_id]));
  }

  std::cout << "THREADS CREATED" << std::endl;
  for (pthread_t t_id = 0; t_id < kThreadNum; ++t_id) {
    ::pthread_join(threads[t_id], NULL);
  }

  // processing final sum
  double finalSum = (threadData[0].sum +
                     threadData[1].sum +
                     threadData[2].sum +
                     threadData[3].sum);
    std::cout << "------------------------------" << std::endl;
    std::cout << "FINAL SUM: " << finalSum << std::endl;

  // destory shared memory
  ::munmap(store_, sizeof(store_));
  ::shm_unlink(SHMPATH);
}

void* processThread(void* input) {
  struct ThreadData* data = reinterpret_cast<struct ThreadData*>(input);
  std::string line;
  for (size_t i = 0; i < kMemFourthSize; ++i) {
    if ((store_->buffer[data->memArrayNumber][i] == DomainSocket::kUS ||
        store_->buffer[data->memArrayNumber][i] == DomainSocket::kEoT) &&
        line.size() > 0) {
          data->sum = (data->sum + processEquation(line));
          line = "";
          ++data->lines;
       } else if (store_->buffer[data->memArrayNumber][i]
                  != DomainSocket::kNULL) {
        line += store_->buffer[data->memArrayNumber][i];
       }
  }
  // Printing each thread out
  std::cout << " THREAD " << data->memArrayNumber
            << " LINES: " << data->lines
            << " TOTAL: " << data->sum << std::endl;
  return nullptr;
}

double AddNumbers(double a, double b) {
  return a + b;
}

double SubtractNumbers(double a, double b) {
  return a - b;
}

double MultiplyNumbers(double a, double b) {
  return a * b;
}

double DivideNumbers(double a, double b) {
  return a / b;
}

bool IsOperator(std::string arg) {
  return arg == "+" || arg == "-" || arg == "x" || arg == "/";
}

double processEquation(std::string line) {
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
  return numbers[0];
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
