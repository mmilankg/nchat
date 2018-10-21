#ifndef CONNECTION_H
#define CONNECTION_H

class Connection {
    // observer
    /* Similar to Acceptor, the observer here is the Server object. */
    Observer * pObserver;
    Socket *   pClientSocket;

public:
    Connection(Observer * pObs, Socket * pSock) : pObserver(pObs), pClientSocket(pSock) {}
    ~Connection() { delete pClientSocket; }

    void     tx(const std::vector<char> & message) { pObserver->react(message); }
    void     rx(const std::vector<char> & message) { /* DBG: implement later */}
    Socket * getSocket() { return pClientSocket; }
    int      getSfd() { return pClientSocket->getSfd(); }
};

#endif // CONNECTION_H
