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
#define CLIENT_SEM "ctaks_client"

#define SHMPATH "csvshmem"



template <std::size_t BufferSize>
struct SharedMemoryStore {
  static const size_t kBuffSize = 291000;
  std::size_t buffer_size[4];
  //std::size_t buffer_size;
  char buffer[BufferSize];
  std::string strBuffer;
};

void quit();


#endif  // _PROJ3_SHM_H_