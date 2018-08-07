//
// Created by Edmund Mok on 8/7/18.
//

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
  if (value == 1) cv.notify_one(); // cv.notify_all();
}