#ifndef CONNECTION_H
#define CONNECTION_H

#include "message.h"
#include "observer.h"
#include "socket.h"

class Connection {
    // observer
    /* Similar to Acceptor, the observer here is the Server object. */
    Observer * pObserver;
    Socket *   pSocket;

public:
    Connection(Observer * pObs, Socket * pSock) : pObserver(pObs), pSocket(pSock) {}
    ~Connection() { delete pSocket; }

    void receive();
    void transmit(const std::vector<char> & message) { pSocket->send(message); }
    void transmit(MessageType messageType, const std::vector<char> & messageContent);
    void transmit(MessageType messageType, const std::string & messageContent);
    void transmit(MessageType messageType, int messageContent);

    Socket * getSocket() { return pSocket; }
    int      getSfd() { return pSocket->getSfd(); }
};

#endif // CONNECTION_H
