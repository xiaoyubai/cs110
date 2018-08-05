/**
 * File: stsh.cc
 * -------------
 * Defines the entry point of the stsh executable.
 */

#include "stsh-parser/stsh-parse.h"
#include "stsh-parser/stsh-readline.h"
#include "stsh-parser/stsh-parse-exception.h"
#include "stsh-signal.h"
#include "stsh-job-list.h"
#include "stsh-job.h"
#include "stsh-process.h"
#include <assert.h>
#include <cstring>
#include <iostream>
#include <string>
#include <algorithm>
#include <fcntl.h>
#include <unistd.h>  // for fork
#include <signal.h>  // for kill
#include <sys/wait.h>
using namespace std;

static STSHJobList joblist; // the one piece of global data we need so signal handlers can access it

static void executeFgCommand(const pipeline& pipeline);

/**
 * Function: handleBuiltin
 * -----------------------
 * Examines the leading command of the provided pipeline to see if
 * it's a shell builtin, and if so, handles and executes it.  handleBuiltin
 * returns true if the command is a builtin, and false otherwise.
 */
static const string kSupportedBuiltins[] = {"quit", "exit", "fg", "bg", "slay", "halt", "cont", "jobs"};
static const size_t kNumSupportedBuiltins = sizeof(kSupportedBuiltins)/sizeof(kSupportedBuiltins[0]);
static bool handleBuiltin(const pipeline& pipeline) {
  const string& command = pipeline.commands[0].command;
  auto iter = find(kSupportedBuiltins, kSupportedBuiltins + kNumSupportedBuiltins, command);
  if (iter == kSupportedBuiltins + kNumSupportedBuiltins) return false;
  size_t index = iter - kSupportedBuiltins;

  switch (index) {
  case 0:
  case 1: exit(0);
  case 2: executeFgCommand(pipeline); break;
  case 7: cout << joblist; break;
  default: throw STSHException("Internal Error: Builtin command not supported."); // or not implemented yet
  }
  
  return true;
}

static void handleSigChld(int sig) {
  /**
   * This assumes only a single foreground process / job.
   */
  int status;
  pid_t pid = waitpid(-1, &status, WUNTRACED | WCONTINUED);
  STSHJob& job = joblist.getJobWithProcess(pid);
  if (WIFSTOPPED(status)) {
    job.getProcess(pid).setState(kStopped);
  } else if (WIFCONTINUED(status)) {
    job.getProcess(pid).setState(kRunning);
  } else {
    assert(WIFEXITED(status));
    job.getProcess(pid).setState(kTerminated);
  }

  joblist.synchronize(job);
}

static void handleSigInt(int sig) {
  if (!joblist.hasForegroundJob()) return;
  STSHJob& fg = joblist.getForegroundJob();
  kill(-fg.getGroupID(), SIGINT);
}

static void handleSigTstp(int sig) {
  if (!joblist.hasForegroundJob()) return;
  STSHJob& fg = joblist.getForegroundJob();
  kill(-fg.getGroupID(), SIGTSTP);
}

/**
 * Function: installSignalHandlers
 * -------------------------------
 * Installs user-defined signals handlers for four signals
 * (once you've implemented signal handlers for SIGCHLD, 
 * SIGINT, and SIGTSTP, you'll add more installSignalHandler calls) and 
 * ignores two others.
 */
static void installSignalHandlers() {
  installSignalHandler(SIGQUIT, [](int sig) { exit(0); });
  installSignalHandler(SIGTTIN, SIG_IGN);
  installSignalHandler(SIGTTOU, SIG_IGN);
  installSignalHandler(SIGCHLD, handleSigChld);
  installSignalHandler(SIGINT, handleSigInt);
  installSignalHandler(SIGTSTP, handleSigTstp);
}

static void executeFgCommand(const pipeline& pipeline) {
  command cmd = pipeline.commands.front();
  size_t numToks;
  for (numToks=0; numToks < kMaxArguments; numToks++) {
    if (cmd.tokens[numToks] == NULL) break;
  }

  if (numToks < 1) {
    throw STSHException("fg takes at least one argument.");
  }

  try {
    // check for proper integer
    int jobNum = stoi(cmd.tokens[0]);
    // check for unsigned int
    if (jobNum <= 0) throw STSHException("Job number must be positive.");
    // check for valid job in joblist
    if (!joblist.containsJob(jobNum)) throw STSHException("Job is not a valid job number.");
    // forward SIGCONT to job
    STSHJob& job = joblist.getJob(jobNum);
    kill(-job.getGroupID(), SIGCONT);
    job.setState(kForeground);
    // print command to terminal
  } catch (invalid_argument& ia) {
    throw STSHException("Job number must be an integer.");
  }

}

// Helper function to construct the argv array for execvp based on the command
// and tokens. Assumes that argv has sufficient space.
static void buildArgv(command& cmd, char *argv[]) {
  argv[0] = cmd.command;
  for (size_t i=0; i<kMaxArguments; i++) {
    argv[i+1] = cmd.tokens[i];
    if (cmd.tokens[i] == NULL) break;
  }
}

/**
 * Function: createJob
 * -------------------
 * Creates a new job on behalf of the provided pipeline.
 */
static void createJob(const pipeline& p) {
  /**
   * Assuming only single command!
   */
  pid_t pid;
  command cmd = p.commands.front();
  if ((pid = fork()) == 0) {
    setpgid(0, 0);
    char *argv[kMaxArguments];
    buildArgv(cmd, argv);
    execvp(argv[0], argv);
  } else {
    // TODO: Block signals while adding this to job list.
    STSHJob& job = joblist.addJob(kForeground);
    STSHProcess proc(pid, cmd);
    job.addProcess(proc);
  }
}

static void suspendForForeground() {
  sigset_t oldMask, extraMask;
  sigemptyset(&extraMask);
  sigaddset(&extraMask, SIGCHLD);
  sigprocmask(SIG_BLOCK, &extraMask, &oldMask);
  while (joblist.hasForegroundJob())
    sigsuspend(&oldMask);
  sigprocmask(SIG_UNBLOCK, &extraMask, NULL);
}

/**
 * Function: main
 * --------------
 * Defines the entry point for a process running stsh.
 * The main function is little more than a read-eval-print
 * loop (i.e. a repl).  
 */
int main(int argc, char *argv[]) {
  pid_t stshpid = getpid();
  installSignalHandlers();
  rlinit(argc, argv);
  while (true) {
    string line;
    if (!readline(line)) break;
    if (line.empty()) continue;
    try {
      pipeline p(line);
      bool builtin = handleBuiltin(p);
      if (!builtin) createJob(p);
      suspendForForeground();
    } catch (const STSHException& e) {
      cerr << e.what() << endl;
      if (getpid() != stshpid) exit(0); // if exception is thrown from child process, kill it
    }
  }

  return 0;
}
