/**
 * File: thread-pool.cc
 * --------------------
 * Presents the implementation of the ThreadPool class.
 */

#include "thread-pool.h"
using namespace std;

ThreadPool::ThreadPool(size_t numThreads) : wts(numThreads) {
  dt = thread([this](){
    dispatcher();
  });

  for (size_t workerID = 0; workerID < numThreads; workerID++) {
    wts[workerID] = thread([this](size_t workerID){
      worker(workerID);
    }, workerID);
  }
}
void ThreadPool::schedule(const function<void(void)>& thunk) {}
void ThreadPool::wait() {}
ThreadPool::~ThreadPool() {}
