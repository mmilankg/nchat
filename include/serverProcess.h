#ifndef SERVERPROCESS_H
#define SERVERPROCESS_H

#include "process.h"
#include "socket.h"
#include "user.h"
#include <fstream>
#include <random>
#include <string>

class ServerProcess : public Process {
  // name of the file with user data (name, password, contacts... (conversation history?))
  std::string usersFileName;
  std::fstream usersFile;
  // vector of users
  std::vector<User> users;

  // listening socket
  Socket listeningSocket;
  // list of client sockets
  std::vector<Socket*> clientSockets;

  // random number generator for randomly choosing salt from the list of 64 possible characters
  std::default_random_engine saltGenerator;
  std::uniform_int_distribution<int> saltDistribution;

  public:
  ServerProcess();
  ~ServerProcess() { }
  int run();

  private:
  void checkUsername(const Socket* clientSocket, const std::string& username) const;
  void checkUser(const Socket* clientSocket, const char* buffer);
  void logoutUser(const Socket* clientSocket);
  void addUser(const Socket* clientSocket, const char* userData);
};

#endif	// SERVERPROCESS_H
