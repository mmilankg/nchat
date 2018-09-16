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
    char* msg = new char[message.getLength() + 1];
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

      /*
       * Inform the distributor process about the user.  Pack the name, username
       * and the password into the message and put it into the message queue.
       */
      // Add 3 for null-characters after each component.
      int messageLength = name.length() + username.length() + password.length() + 3;
      // Initialize the buffer with null-characters.
      char* buffer = new char[messageLength]();
      // Copy message components into the buffer.
      name.copy(buffer, name.length(), 0);
      username.copy(buffer + name.length() + 1, username.length(), 0);
      password.copy(buffer + name.length() + username.length() + 2, password.length(), 0);
      pMessageQueue->push_back(Message(pid, distributorPid, mAddUser, messageLength, buffer));
      delete []buffer;
    }
  }
}

int ClientDedicatedProcess::loginUser() {
  /*
   * The server now needs only the username and password.  As in the signupUser() function,
   * the password is still received in plaintext, which is then passed to the distributor
   * process for checking against its database.
   */
  std::string username, password;
  rCommunicationSocket.recv(username);
  // Insert sending to achieve synchronization with the client.
  rCommunicationSocket.send(0);
  rCommunicationSocket.recv(password);
  rCommunicationSocket.send(0);
  // Pack the username and the password into the message before sending to the queue.
  /* DBG: This is done at least twice, so it could be replaced by a function that would
   * receive a set of strings (e.g. a vector) as an input, and return a memory address
   * of a buffer.  This function could be a private function of the Process class as
   * all of these processes will use it.  Another possibility is to make it as a general
   * function so that messages for socket transfer can be prepared this way from the
   * client-side. */
  int messageLength = username.length() + password.length() + 2;
  char* buffer = new char[messageLength]();
  username.copy(buffer, username.length(), 0);
  password.copy(buffer + username.length() + 1, password.length(), 0);
  // Send to the distributor process for check.
  pMessageQueue->push_back(Message(pid, distributorPid, mCheckUser, messageLength, buffer));
  delete []buffer;

  // Wait for the distributor process response (OK or NOK).
  bool responseReceived = false;
  while (!responseReceived) {
    if (pMessageQueue->size() == 0)
      continue;

    Message& message = pMessageQueue->front();
    pid_t messageRecipient = message.getRecipient();
    if (messageRecipient == pid) {
      responseReceived = true;
      MessageType messageType = message.getType();
      pid_t messageSender = message.getSender();
      char* msg = new char[message.getLength() + 1];
      message.read(msg);
      message.releaseContents();
      pMessageQueue->pop_front();

      if ((messageType != mCheckUser) || (messageSender != distributorPid))
	std::cout << "Wrong message received!" << std::endl;
      if (std::string(msg) == "OK")
	rCommunicationSocket.send(0);
      else if (std::string(msg) == "NOK")
	rCommunicationSocket.send(1);
      else
	// Message sent for the unknown user; when the username was not found in the database.
	rCommunicationSocket.send(2);
      delete []msg;
    }
  }
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
