/**
 * File: trace.cc
 * ----------------
 * Presents the implementation of the trace program, which traces the execution of another
 * program and prints out information about ever single system call it makes.  For each system call,
 * trace prints:
 *
 *    + the name of the system call,
 *    + the values of all of its arguments, and
 *    + the system calls return value
 */

#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <unistd.h> // for fork, execvp
#include <signal.h>
#include <string.h> // for memchr, strerror
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/wait.h>
#include "trace-options.h"
#include "trace-error-constants.h"
#include "trace-system-calls.h"
#include "trace-exception.h"
using namespace std;

#define SIGTRAP2 (SIGTRAP | 0x80)

int main(int argc, char *argv[]) {
  bool simple = false, rebuild = false;
  int numFlags = processCommandLineFlags(simple, rebuild, argv);
  if (argc - numFlags == 1) {
    cout << "Nothing to trace... exiting." << endl;
    return 0;
  }

  int trueArgvIndex = numFlags+1;
  map<int, std::string> systemCallNumbers;
  map<std::string, int> systemCallNames;
  map<std::string, systemCallSignature> systemCallSignatures;
  compileSystemCallData(systemCallNumbers, systemCallNames, systemCallSignatures, rebuild);

  int pid = fork();
  if (pid < 0) {
    fprintf(stderr, "Failed to fork.\n");
    return 1;
  }

  if (pid == 0) {
    ptrace(PTRACE_TRACEME);
    raise(SIGSTOP);
    execvp(argv[trueArgvIndex], argv+trueArgvIndex);
  } else {
    waitpid(pid, NULL, 0);
    ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_TRACESYSGOOD);
    ptrace(PTRACE_SYSCALL, pid, 0, 0);
    int status;
    while (true) {
      waitpid(pid, &status, 0);
      if (WIFEXITED(status)) {
        break;
      } else if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP2) {
        int syscall = ptrace(PTRACE_PEEKUSER, pid, ORIG_RAX * sizeof(long));
        ptrace(PTRACE_SYSCALL, pid, 0, 0);
        waitpid(pid, NULL, 0);
        long retval = ptrace(PTRACE_PEEKUSER, pid, RAX * sizeof(long));
        cout << "syscall(" << syscall << ") = " << retval << endl;
        ptrace(PTRACE_SYSCALL, pid, 0, 0);
      } else {
        ptrace(PTRACE_SYSCALL, pid, 0, 0);
      }
    }

    cout << "Program exited normally with status " << WEXITSTATUS(status) << endl;
    return WEXITSTATUS(status);
  }


  return 0;
}
