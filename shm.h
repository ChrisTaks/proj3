// Copyright 2024 Chris Taks

#ifndef _PROJ3_SHM_H_
#define _PROJ3_SHM_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <semaphore.h>
#include <pthread.h>
#include <string>
#include <signal.h>
#include <cstddef>  // size_t

#define SERVER_SEM "ctaks_server"
const char* kServerSem = "ctaks_server";
const char* kClientSem = "ctaks_client";

#define CLIENT_SEM "ctaks_client"

#define SHMPATH "csvshmem"

const int kArraySize = 4;
const int kMemFourthSize = (1 << 19);

struct SharedMemoryStore {
  std::size_t buffer_size[kArraySize];
  char buffer[kArraySize][kMemFourthSize];
};

void quit();

#endif  // _PROJ3_SHM_H_