#include "listeningProcess.h"
#include <fstream>

int ListeningProcess::run() {
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
