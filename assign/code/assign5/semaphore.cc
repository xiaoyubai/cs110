//
// Created by Edmund Mok on 8/7/18.
//

#include <iostream>
#include "ostreamlock.h"
#include <boost/thread/thread.hpp>
#include "semaphore.h"

using namespace std;

void semaphore::wait() {
  lock_guard<mutex> lg(m);
  if (value == 0) cv.wait(m, [this] { return value > 0; }); // guard against spurious wakeup
  value--;
}

void semaphore::signal() {
  lock_guard<mutex> lg(m);
  value++;
  if (value == 1) cv.notify_all(); // cv.notify_one();
}

void semaphore::signal(signal_condition) {
  boost::this_thread::at_thread_exit([this] {
      this->signal();
  });
}