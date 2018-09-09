#include "distributorProcess.h"
#include "listeningProcess.h"

int main() {
  /*
   * Before the listening process starts blocking on the accept call,
   * the program needs to spawn anothe process whose purpose is to
   * communicate with client-dedicated processes: distributing messages
   * between clients (client processes), finding users for connection
   * requests, checking user passwords...  As one of the main roles of
   * this process is distributing the messages between client-oriented
   * processes, the class whose object performs this role is named
   * DistributorProcess.
   */
  pid_t pid = fork();
  if (pid == 0) {
    DistributorProcess distributorProcess;
    distributorProcess.run();
  }
  else {
    /*
     * The listening process opens a socket that listens to connection
     * requests from clients.  Each connection then spawns a new
     * process.
     */
    ListeningProcess listeningProcess(pid);
    listeningProcess.run();
  }
}
