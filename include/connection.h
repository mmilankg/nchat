#ifndef CONNECTION_H
#define CONNECTION_H

#include "observer.h"
#include "socket.h"

class Connection {
    // observer
    /* Similar to Acceptor, the observer here is the Server object. */
    Observer * pObserver;
    Socket *   pClientSocket;

public:
    Connection(Observer * pObs, Socket * pSock) : pObserver(pObs), pClientSocket(pSock) {}
    ~Connection() { delete pClientSocket; }

    void receive();
    void transmit(const std::vector<char> & message) { /* DBG: implement later */}

    Socket * getSocket() { return pClientSocket; }
    int      getSfd() { return pClientSocket->getSfd(); }
};

#endif // CONNECTION_H
