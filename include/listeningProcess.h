#ifndef LISTENINGPROCESS_H
#define LISTENINGPROCESS_H

#include "process.h"
#include "socket.h"
#include "clientDedicatedProcess.h"
#include <list>
#include <string>

class ListeningProcess : public Process {
  // process ID of the distributor process -- a process for coordinating client-oriented processes
  pid_t distributorPid;
  // socket for listening to incoming connection requests from clients
  Socket listeningSocket;
  // list of all processes opened with different clients
  //std::list<ClientDedicatedProcess> clientProcessesList;
  // name of the file with user data
  //std::string usersFileName;
  // object for encrypting user passwords
  //Encryptor hashingObject;
  public:
  ListeningProcess(pid_t distributor) :
    distributorPid(distributor),
    listeningSocket("")
    //usersFileName("nchatUsers")
  { };
  virtual ~ListeningProcess() { };
  /*
   * Central function of the listening process: listen to new connection
   * requests and spawn a new process for each incoming client connection.
   */
  int run();
};

#endif	// LISTENINGPROCESS_H
