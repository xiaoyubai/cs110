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
#include <sstream>
#include <string.h> // for memchr, strerror
#include "string-utils.h"
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/wait.h>
#include "trace-options.h"
#include "trace-error-constants.h"
#include "trace-system-calls.h"
#include "trace-exception.h"
using namespace std;

#define SIGTRAP2 (SIGTRAP | 0x80)

typedef int regtype;

static const regtype argRegisters[] = {RDI, RSI, RDX, R10, R8, R9};
static const set<string> voidStarReturns = {"brk", "sbrk", "mmap"};

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
  map<int, std::string> errorConstants;
  compileSystemCallData(systemCallNumbers, systemCallNames, systemCallSignatures, rebuild);
  compileSystemCallErrorStrings(errorConstants);

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
        int sysCallNum = ptrace(PTRACE_PEEKUSER, pid, ORIG_RAX * sizeof(long));
        const string sysCallName = systemCallNumbers[sysCallNum];

        if (!simple) {
          vector<string> stringArgs;
          if (systemCallSignatures.find(sysCallName) == systemCallSignatures.end()) {
            stringArgs.push_back("<signature-information-missing>");
          }
          else {
            const systemCallSignature sysCallSig = systemCallSignatures[sysCallName];
            for (size_t i=0; i<sysCallSig.size(); i++) {
              long regVal = ptrace(PTRACE_PEEKUSER, pid, argRegisters[i] * sizeof(long));
              switch (sysCallSig[i]) {
                case SYSCALL_INTEGER:
                  stringArgs.push_back(to_string((int) regVal));
                  break;
                case SYSCALL_STRING: {
                  if ((void *) regVal == NULL) {
                    stringArgs.push_back("NULL");
                    break;
                  }
                  // regVal points to a null terminated string
                  string res = "\"";
                  while (true) {
                    bool done = false;
                    long regStr = ptrace(PTRACE_PEEKDATA, pid, regVal);
                    for (size_t j=0; j<sizeof(long); j++) {
                      char ch = ((char *) &regStr)[j];
                      res += ch;
                      if (ch == '\0') {
                        done = true;
                        break;
                      }
                    }
                    regVal += sizeof(long);
                    if (done) break;
                  }
                  res += "\"";
                  stringArgs.push_back(res);
                  break;
                }
                case SYSCALL_POINTER: {
                  stringstream ss;
                  if ((void *) regVal != NULL)
                    ss << (void *) regVal;
                  else
                    ss << "NULL";
                  stringArgs.push_back(ss.str());
                  break;
                }
                case SYSCALL_UNKNOWN_TYPE:
                  fprintf(stderr, "Syscall with unknown param type\n");
                  return 2;
              }
            }
          }
          cout << sysCallName << "(" << join(stringArgs, ", ") << ") = ";
        } else {
          cout << "syscall(" << sysCallNum << ") = ";
        }

        ptrace(PTRACE_SYSCALL, pid, 0, 0);
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
          cout << "<no return>" << endl;
          break;
        }

        long retval = ptrace(PTRACE_PEEKUSER, pid, RAX * sizeof(long));
        if (simple) {
          cout << retval;
        } else {
          if (retval < 0) {
            retval = abs(retval);
            cout << "-1";
            if (!simple && errorConstants.find(retval) != errorConstants.end())
              cout << " " << errorConstants[retval] << " (" << strerror(retval) << ")";
          } else if (voidStarReturns.find(sysCallName) != voidStarReturns.end()) {
            cout << (void *) retval;
          } else {
            cout << (int) retval;
          }
        }
        cout << endl;
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
