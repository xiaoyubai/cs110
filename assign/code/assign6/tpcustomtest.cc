/**
 * File: tpcustomtest.cc
 * ---------------------
 * Unit tests *you* write to exercise the ThreadPool in a variety
 * of ways.
 */

#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include <functional>
#include <cstring>

#include <sys/types.h> // used to count the number of threads
#include <unistd.h>    // used to count the number of threads
#include <dirent.h>    // for opendir, readdir, closedir

#include "thread-pool.h"
#include <thread>
#include "ostreamlock.h"
using namespace std;

static void singleThreadNoWaitTest() {
  ThreadPool pool(4);
  pool.schedule([] {
    cout << oslock << "This is a test." << endl << osunlock;
  });
  this_thread::sleep_for(std::chrono::milliseconds(1000));
}

static void singleThreadSingleWaitTest() {
  ThreadPool pool(4);
  pool.schedule([] {
    cout << oslock << "This is a test." << endl << osunlock;
    this_thread::sleep_for(std::chrono::milliseconds(1000));
  });
  pool.wait();
}

static void noThreadsDoubleWaitTest() {
  ThreadPool pool(4);
  pool.wait();
  pool.wait();
}

static void reuseThreadPoolTest() {
  ThreadPool pool(4);
  for (size_t i = 0; i < 16; i++) {
    pool.schedule([] {
      cout << oslock << "This is a test." << endl << osunlock;
      this_thread::sleep_for(std::chrono::milliseconds(50));
    });
  }
  pool.wait();
  pool.schedule([] {
    cout << oslock << "This is a code." << endl << osunlock;
    this_thread::sleep_for(std::chrono::milliseconds(1000));
  });
  pool.wait();
}

static void poolLimitAndOrderTest() {
  ThreadPool pool(2);
  for (size_t i = 0; i < 20; i++) {
    pool.schedule([i]{
      this_thread::sleep_for(std::chrono::milliseconds(100 * (i/2)));
      cout << oslock << "Thread " << i << " of Group " << (i / 2) << endl << osunlock;
    });
  }
  pool.wait();
}

static void waitAfterThreadsFinishTest() {
  ThreadPool pool(4);
  for (size_t i=0; i <4; i++) {
    pool.schedule([i]{
      this_thread::sleep_for(std::chrono::milliseconds(100));
      cout << oslock << "Thread " << i << " done." << endl << osunlock;
    });
  }
  this_thread::sleep_for(std::chrono::milliseconds(200));
  cout << oslock << "Waiting now." << endl << osunlock;
  pool.wait();
}

struct testEntry {
  string flag;
  function<void(void)> testfn;
};

static void buildMap(map<string, function<void(void)>>& testFunctionMap) {
  testEntry entries[] = {
    {"--single-thread-no-wait", singleThreadNoWaitTest},
    {"--single-thread-single-wait", singleThreadSingleWaitTest},
    {"--no-threads-double-wait", noThreadsDoubleWaitTest},
    {"--reuse-thread-pool", reuseThreadPoolTest},
    {"--pool-limit-and-order", poolLimitAndOrderTest},
    {"--wait-after-threads-finish", waitAfterThreadsFinishTest}
  };

  for (const testEntry& entry: entries) {
    testFunctionMap[entry.flag] = entry.testfn;
  }
}

static void executeAll(const map<string, function<void(void)>>& testFunctionMap) {
  for (const auto& entry: testFunctionMap) {
    cout << entry.first << ":" << endl;
    entry.second();
  }
}

int main(int argc, char **argv) {
  if (argc != 2) {
    cout << "Ouch! I need exactly two arguments." << endl;
    return 0;
  }
  
  map<string, function<void(void)>> testFunctionMap;
  buildMap(testFunctionMap);
  string flag = argv[1];
  if (flag == "--all") {
    executeAll(testFunctionMap);
    return 0;
  }
  auto found = testFunctionMap.find(argv[1]);
  if (found == testFunctionMap.end()) {
    cout << "Oops... we don't recognize the flag \"" << argv[1] << "\"." << endl;
    return 0;
  }
  
  found->second();
  return 0;
}
