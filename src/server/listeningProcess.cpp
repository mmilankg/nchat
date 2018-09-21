#include "listeningProcess.h"
#include <signal.h>
#include <wait.h>
#include <fstream>

void signalHandler(int signal, siginfo_t* signalInfo, void* context) {
  int status;
  wait(&status);
}

int ListeningProcess::run() {
  /*
   * Set up a handler for SIGCHLD signals that are sent to the process when
   * client-dedicated processes finish.  This should help with removing zombie
   * processes.
   */
  struct sigaction signalAction;
  // pointer to the handler function
  /* DBG: Ignore SIGCHLD in the first instance; intercept with a proper handler later! */
  //signalAction.sa_sigaction = &signalHandler;
  //signalAction.sa_flags = SA_SIGINFO;
  signalAction.sa_handler = SIG_DFL;
  signalAction.sa_flags = SA_NOCLDWAIT;
  sigaction(SIGCHLD, &signalAction, 0);
  /*
   * Listen to incoming connection requests.  When one is received, accept it and
   * spawn a new process to deal with it.
   */
  while (true) {
    Socket& clientDedicatedSocket = listeningSocket.acceptConnection();
    pid_t pid = fork();
    if (pid == 0) {
      ClientDedicatedProcess clientProcess(distributorPid, clientDedicatedSocket);
      clientProcess.run();
    }
  }
}
