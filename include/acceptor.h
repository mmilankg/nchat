#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include "observer.h"
#include "socket.h"

/*
 * An object of the Acceptor class is created by the server in order to listen to incoming connection requests from the
 * clients.  For an accepted connection, it should produce a connected socket, which is then returned to the server
 * which uses it to construct an object of the Connection class.
 */
class Acceptor {
    // observer of the acceptor
    /*
     * This is going to be the only observer, and it's going to be a pointer to the server itself, so it could have been
     * created as Server *, but this approach with upcasting the pointer to the base Observer class seems to be more
     * applicable for a general case, and online examples generally work with a whole vector of pointers to objects
     * derived from the base Observer class.
     */
    Observer * pObserver;
    Socket     listeningSocket;

public:
    /* Passing an empty string sets up a listening socket. */
    Acceptor(Observer * pObs) : pObserver(pObs), listeningSocket("") {}
    ~Acceptor() {}
    void acceptConnection();
    int  getSfd() { return listeningSocket.getSfd(); }

private:
    void sendSocket(Socket * pClientSocket) { pObserver->react(pClientSocket); }
};

#endif // ACCEPTOR_H
