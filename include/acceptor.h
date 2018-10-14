#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include "serverProcess.h"
#include "socket.h"

/*
 * An object of the Acceptor class is created by the server process in order to listen to incoming connection requests
 * from the clients.  For an accepted connection, it should produce a connected socket, which is then returned to the
 * server process which uses it to construct an object of the Connection class.
 */
class Acceptor {
    // observer of the acceptor
    ServerProcess * pServerProcess;
    Socket *        pListeningSocket;

public:
    Acceptor() : pServerProcess(0), pListeningSocket(0) {}
    Acceptor(ServerProcess * pServProc, Socket * pListenSock) : pServerProcess(pServProc), pListeningSocket(pListenSock)
    {
    }
    ~Acceptor() {}
    void acceptConnection();

private:
    void sendSocket(Socket * pClientSocket) { pServerProcess->createConnection(pClientSocket); }
};

#endif // ACCEPTOR_H
