#ifndef SERVER_H
#define SERVER_H

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

    // listening socket
    /* DBG: Perhaps now that the Acceptor object will be created, it would be better to have the listening socket as a
     * member of the Acceptor object. */
    Socket listeningSocket;
    // list of client sockets
    std::vector<Socket *> clientSockets;

    // random number generator for randomly choosing salt from the list of 64 possible characters
    std::default_random_engine         saltGenerator;
    std::uniform_int_distribution<int> saltDistribution;

public:
    Server();
    ~Server() {}
    int  run();
    void createConnection(Socket * pSocket);
    // overriding the react() function of the Observer base class
    void react(void * pSocket) { createConnection(static_cast<Socket *>(pSocket)); }
    /*
     * DBG: I probably shouldn't be casting pointers.  It doesn't seem like a good practice, and I'm not even sure if
     * it's allowed in this context.  The logic here was to derive Server from Observer and use the Observer design
     * pattern for connecting with the Acceptor.  In order to make the Observer class more generic, its react() function
     * is made to accept void * pointer rather than Socket *.  But the overriding function should then also receive a
     * void * pointer, which then has to be converted to Socket * in order to pass it into the createConnection()
     * function.
     */

private:
    void signup(Socket * clientSocket, const std::vector<std::string> & userDetails);
    void login(Socket * clientSocket, const std::vector<std::string> & userDetails);
    void logout(Socket * clientSocket, const std::string & username);

    int  checkUsername(const std::string & username) const;
    void addUser(Socket * clientSocket, const std::vector<std::string> & userDetails);
    void findUser(Socket * clientSocket, const std::string & username);

    void bufferToStrings(char * buffer, int bufferLength, std::vector<std::string> & strings) const;
};

#endif // SERVER_H
