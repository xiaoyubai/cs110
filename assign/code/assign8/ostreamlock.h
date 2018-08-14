#include <ostream>
#include <iostream>
#include <mutex>
#include <memory>
#include <map>
using namespace std;

static mutex mapLock;
static map<ostream *, unique_ptr<mutex>> streamLocks;

static ostream& oslock(ostream& os) {
  ostream *ostreamToLock = &os;
  if (ostreamToLock == &cerr) ostreamToLock = &cout;
  mapLock.lock();
  unique_ptr<mutex>& up = streamLocks[ostreamToLock];
  if (up == nullptr) {
    up.reset(new mutex);
  }
  mapLock.unlock();
  up->lock();
  return os;
}

static ostream& osunlock(ostream& os) {
  ostream *ostreamToLock = &os;
  if (ostreamToLock == &cerr) ostreamToLock = &cout;
  mapLock.lock();
  auto found = streamLocks.find(ostreamToLock);
  mapLock.unlock();
  if (found == streamLocks.end())
    throw "unlock inserted into stream that has never been locked.";
  unique_ptr<mutex>& up = found->second;
  up->unlock();
  return os;
}
