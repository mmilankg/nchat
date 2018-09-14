#include "commands.h"
#include "message.h"
#include "clientDedicatedProcess.h"
#include <unistd.h>
#include <iostream>
#include <sstream>

ClientDedicatedProcess::ClientDedicatedProcess(pid_t distributor, const Socket& socket) :
  distributorPid(distributor),
  rCommunicationSocket(socket)
{
  // Find the process ID of this process.
  pid = getpid();
  // Open shared memory segments created by the distributor process.
  pUserSegment = new boost::interprocess::managed_shared_memory(boost::interprocess::open_only, "userVectorMemory");
  pMessageSegment = new boost::interprocess::managed_shared_memory(boost::interprocess::open_only, "messageQueueMemory");
  // Find the user vector and the message queue in their shared memory segments.
  pUserVector = pUserSegment->find<UserVector>("userVector").first;
  pMessageQueue = pMessageSegment->find<MessageQueue>("messageQueue").first;
}
int ClientDedicatedProcess::run() {
  int command;
  while (true) {
    rCommunicationSocket.recv(command);
    // Acknowledge receipt of the command.
    rCommunicationSocket.send(0);
    switch (command) {
      case cSignup : signupUser(); break;
      case cLogin : loginUser(); break;
      case cLogout : logoutUser(); break;
      case cQuit : quit(); break;
      default : ;
    }
  }
}

int ClientDedicatedProcess::signupUser() {
  /*
   * The server receives the real name, username and the password.
   * The password here is received in plaintext and then encrypted
   * and stored in a file with other user data.  Ideally, it should
   * be encrypted by the client process, so that it is not transmitted
   * in plaintext.
   * As in the client program, recv() and send() calls are alternated
   * in order to keep the connection synchronized.
   */
  std::string name, username, password;
  rCommunicationSocket.recv(name);
  rCommunicationSocket.send(0);
  rCommunicationSocket.recv(username);
  rCommunicationSocket.send(0);
  rCommunicationSocket.recv(password);
  rCommunicationSocket.send(0);
  /*
   * Send the username to the distributor process to check if it has already
   * been used by someone else.  Create a message with message type mCheckUsername,
   * length set by the number of characters in the username and contents filled
   * by the username.
   * Construct the message in the queue, which places it directly in the shared memory.
   *
   * DBG: This approach still creates a transient copy that invokes constructor and
   * destructor.  Releasing the character allocation in the distructor would then
   * destroy the message contents!  It would be good if the message could be constructed
   * directly in the queue.  Alternatively, it could be placed in the shared memory, and
   * store the pointer or offset pointer in the queue.
   */
  pMessageQueue->push_back(Message(pid, distributorPid, mCheckUsername, username.length(), username.c_str()));

  // Wait for the server response delivered through the message queue.
  bool responseReceived = false;
  while (!responseReceived) {
    /* DBG: Same code as in the distributor process.  It should be feasible to replace it
     * with a function as both processes are derived from the base Process class, which
     * could contain this function! */
    /* DBG: This has to be replaced with something more efficient; perhaps some kind of
     * an events-based function or a hook, if such a thing exists for queues! */
    if (pMessageQueue->size() == 0)
      continue;

    Message& message = pMessageQueue->front();

    /* Decode the message. */
    MessageType messageType = message.getType();
    pid_t messageSender = message.getSender();
    pid_t messageRecipient = message.getRecipient();
    char* msg = new char [message.getLength() + 1];
    message.read(msg);
    // Remove the message from the queue if the recepient matches the process ID.
    if (messageRecipient == pid) {
      responseReceived = true;
      // Release the memory allocated for the message contents;
      message.releaseContents();
      // Remove from the queue.
      pMessageQueue->pop_front();

      if ((messageType != mUsernameStatus) || (messageSender != distributorPid))
	/* DBG: Throw an exception if the message type is not the username status response
	 * from the distributor process! */
	std::cout << "Wrong message received!" << std::endl;

      /*
       * The number sent back to the user should indicate the response of
       * the server:
       * 0: the username and the password have been accepted
       * 1: the username has already been taken
       * for later implementation
       * 2: the password is too short
       * 3: the password does not have the right combination of
       *    letters (at least one uppercase and one lowercase,
       *    at least one number, and at least one other character)
       */
      if (std::string(msg) == "OK")
	rCommunicationSocket.send(0);
      else
	rCommunicationSocket.send(1);
    }
  }
}

int ClientDedicatedProcess::loginUser() {
}

int ClientDedicatedProcess::logoutUser() {
}

int ClientDedicatedProcess::findUser() {
}

int ClientDedicatedProcess::textUser() {
}

int ClientDedicatedProcess::callUser() {
}

void ClientDedicatedProcess::quit() {
  delete &rCommunicationSocket;
  _exit(0);
}
