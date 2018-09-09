#ifndef CLIENTDEDICATEDPROCESS_H
#define CLIENTDEDICATEDPROCESS_H

#include "distributorProcess.h"	  // included only for Boost typedefs (there should be a better way to handle that)
#include "process.h"
#include "socket.h"
#include "user.h"
#include <sys/types.h>
#include <boost/interprocess/containers/deque.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <fstream>
#include <list>
#include <string>

class ClientDedicatedProcess : public Process {
  pid_t distributorPid;	  // process ID of the distributor process for communicating among client-oriented processes
  const Socket& rCommunicationSocket;	// reference to a socket for communicating with the remote client
  // pointers to memory segments in which the vector of users and queue of messages will be stored
  boost::interprocess::managed_shared_memory* pUserSegment;
  boost::interprocess::managed_shared_memory* pMessageSegment;
  // pointer to the vector of users and message queue
  UserVector* pUserVector;
  MessageQueue* pMessageQueue;

  /* to be implemented:
  std::list<Contact*> contacts;	// list of all contacts for the user connected at the client end
  */

  public:
  ClientDedicatedProcess(pid_t distributor, const Socket& socket);
  ~ClientDedicatedProcess() {
    delete pUserVector;
    delete pMessageQueue;
    delete pUserSegment;
    delete pMessageSegment;
  }
  int run();
  int signupUser();
  int loginUser();
  int findUser();
  int textUser();
  int callUser();
  int logoutUser();
  void quit();
};

#endif	// CLIENTDEDICATEDPROCESS_H
