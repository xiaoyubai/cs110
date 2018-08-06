/**
 * File: pipeline.c
 * ----------------
 * Presents the implementation of the pipeline routine.
 */

#include "pipeline.h"
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>

void pipeline(char *argv1[], char *argv2[], pid_t pids[]) {
  // fds[0] - read, fds[1] - write
  int fds[2];
  pipe(fds);

  pid_t reader = fork();
  if (reader == -1) {
    fprintf(stderr, "Failed to create reader process.\n");
    return;
  }

  if (!reader) {
    // In child reader process
    // close write end of pipe
    close(fds[1]);

    // exec argv2 using fds[0] as stdin
    dup2(fds[0], STDIN_FILENO);
    close(fds[0]);
    execvp(argv2[0], argv2);
  }

  // In parent pipeline process
  pids[1] = reader;

  // close fds[0] since child reader already has it
  close(fds[0]);

  pid_t writer = fork();
  if (writer == -1) {
    fprintf(stderr, "Failed to create writer process.\n");
    return;
  }

  if (!writer) {
    // In child writer process
    // close read end of pipe
    close(fds[0]);

    // exec argv1 using fds[1] as stdout
    dup2(fds[1], STDOUT_FILENO);
    close(fds[1]);
    execvp(argv1[0], argv1);
  }

  // close fds[1] since child writer already has it
  close(fds[1]);

  // In parent pipeline process again
  pids[0] = writer;
}
