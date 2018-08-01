#include <cassert>
#include <ctime>
#include <cctype>
#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <unordered_map>
#include <sched.h>
#include "subprocess.h"

using namespace std;

struct worker {
  worker() {}
  worker(char *argv[]) : sp(subprocess(argv, true, false)), available(false) {}
  subprocess_t sp;
  bool available;
};

static const size_t kNumCPUs = sysconf(_SC_NPROCESSORS_ONLN);
static vector<worker> workers(kNumCPUs);
static size_t numWorkersAvailable = 0;
static unordered_map<int, int> pids;

static void markWorkersAsAvailable(int sig) {
  while (true) {
    pid_t pid = waitpid(-1, NULL, WUNTRACED | WNOHANG);
    if (pid <= 0) break;
    workers[pids[pid]].available = true;
    cout << "numWorkersAvailable++" << endl;
    numWorkersAvailable++;
    printf("avail! new value = %d\n", numWorkersAvailable);
  }
}

static inline sigset_t getMask() {
  sigset_t mask;
  sigfillset(&mask);
  sigdelset(&mask, SIGCHLD);
  return mask;
}

static /* const */ char *kWorkerArguments[] = {"./factor.py", "--self-halting", NULL};
static void spawnAllWorkers() {
  cout << "There are this many CPUs: " << kNumCPUs << ", numbered 0 through " << kNumCPUs - 1 << "." << endl;
  for (size_t i = 0; i < kNumCPUs; i++) {
    workers[i] = worker(kWorkerArguments);
    pids[workers[i].sp.pid]= i;
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(i, &set);
    sched_setaffinity(workers[i].sp.pid, sizeof(cpu_set_t), &set);
    cout << "Worker " << workers[i].sp.pid << " is set to run on CPU " << i << "." << endl;
  }
}

static size_t getAvailableWorker() {
  sigset_t mask = getMask();
  cout << "current numWorkersAvailable " << numWorkersAvailable << endl;
  if (!numWorkersAvailable) {
    sigsuspend(&mask);
    cout << "woke up from sigsupend!" << endl;
  }
  for (size_t i=0; i<workers.size(); i++) {
    if (workers[i].available) return i;
  }
  return -1;
}

static void broadcastNumbersToWorkers() {
  while (true) {
    string line;
    getline(cin, line);
    cout << "waiting for more lines" << endl;
    if (cin.fail()) break;
    size_t endpos;
    long long num = stoll(line, &endpos);
    if (endpos != line.size()) break;
    cout << "waiting for workers" << endl;
    struct worker& wk = workers[getAvailableWorker()];
    cout << "numWorkersAvailable-- from " << numWorkersAvailable <<  endl;
    numWorkersAvailable--;
//    assert(wk.available);
    wk.available = false;
    kill(wk.sp.pid, SIGCONT);
    string lined = line + "\n";
    write(wk.sp.supplyfd, lined.c_str(), lined.size());
  }
}

static void waitForAllWorkers() {
  sigset_t mask = getMask();
  while (numWorkersAvailable < kNumCPUs)
    sigsuspend(&mask);
  cout << "going to close next, numWorkersAvailable:" <<  numWorkersAvailable << endl;
}

static void closeAllWorkers() {
  signal(SIGCHLD, SIG_DFL);
  for (worker& w: workers) {
    cout << "sup" << endl;
    kill(w.sp.pid, SIGCONT);
    assert(close(w.sp.supplyfd) == 0);
  }

  for (worker& w: workers) {
    cout << "waiting on pid: " << w.sp.pid << endl;
    waitpid(w.sp.pid, NULL, 0);
  }
}

int main(int argc, char *argv[]) {
  try {
    signal(SIGCHLD, markWorkersAsAvailable);
    spawnAllWorkers();
    broadcastNumbersToWorkers();
    waitForAllWorkers();
    closeAllWorkers();
    return 0;
  } catch (const SubprocessException& se) {
    cerr << "Problem encountered while trying to run farm of workers for factorization." << endl;
    cerr << "More details here: " << se.what() << endl;
    return 1;
  } catch (...) { // ... here means catch everything else
    cerr << "Unknown internal error." << endl;
    return 2;
  }
}
