#ifndef SERVER_H
#define SERVER_H

#include "acceptor.h"
#include "connection.h"
#include "observer.h"
#include "socket.h"
#include "user.h"
#include <fstream>
#include <random>
#include <string>
#include <vector>

class Server : public Observer {
    // name of the file with user data (name, password, contacts... (conversation history?))
    std::string  usersFileName;
    std::fstream usersFile;
    // vector of users
    std::vector<User> users;

    // Acceptor object
    Acceptor acceptor;
    // list of client sockets
    // std::vector<Socket *> clientSockets;
    // Connection objects
    std::vector<Connection *> connections;

    // random number generator for randomly choosing salt from the list of 64 possible characters
    std::default_random_engine         saltGenerator;
    std::uniform_int_distribution<int> saltDistribution;

public:
    Server();
    ~Server() {}
    int  run();
    void createConnection(Socket * pSocket);
    // overriding the react() functions of the Observer base class
    void react(Socket * pSocket) { createConnection(pSocket); }
    void react(Connection * pConnection, MessageType messageType, const std::vector<char> & message);

private:
    void signup(Socket * clientSocket, const std::vector<std::string> & userDetails);
    void login(Socket * clientSocket, const std::vector<std::string> & userDetails);
    void logout(Socket * clientSocket, const std::string & username);

    int  checkUsername(const std::string & username) const;
    void addUser(Socket * clientSocket, const std::vector<std::string> & userDetails);
    void findUser(Socket * clientSocket, const std::string & username);

    void bufferToStrings(char * buffer, int bufferLength, std::vector<std::string> & strings) const;
    void bufferToStrings(const std::vector<char> & buffer, std::vector<std::string> & strings) const;
};

#endif // SERVER_H
