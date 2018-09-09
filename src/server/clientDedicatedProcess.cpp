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
  /*
   * The number sent back to the user should indicate the response of
   * the server:
   * 0: the username and the password have been accepted
   * 1: the username has already been taken
   * for later implementation
   * 2: the password is too short
   * 3: the password does not have the right combination of
   *	letters (at least one uppercase and one lowercase,
   *	at least one number, and at least one other character)
   */
  // DBG: send 0 for the time being.
  //rCommunicationSocket.send(0);

  /*
   * Add the user to the file listing all users and their contacts.
   * This should be done with the locking mechanism so that the
   * process has an exclusive access to the file while the write
   * operation is ongoing.  This is left for later implementation.
   */
  /*
  std::ifstream userFile(rUsersFileName.c_str());
  std::string line;
  // Parse the user file line by line.
  while (getline(userFile, line)) {
    std::istringstream ss(line);
    std::string field;
    *
     * Parse the selected line field by field.  Fields are separated
     * by a colon.
     * 1. user id (int)
     * 2. user name (string)
     * 3. encrypted user password (string; first two characters are salt)
     * 4. list of contacts (integers separated by commas)
     *
    int userID;
    std::string username;
    std::string name;
    std::string password;
    std::string contacts;
    getline(ss, field, ':');
    userID = atoi(field.c_str());
    getline(ss, username, ':');
    getline(ss, name, ':');
    getline(ss, password, ':');
    getline(ss, contacts, ':');
    User* pUser = new User(userID, username, name, password, offline);
    users.push_back(pUser);
  }
  */
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
