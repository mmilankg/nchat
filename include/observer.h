#ifndef OBSERVER_H
#define OBSERVER_H

#include "message.h"
#include <vector>

class Connection;
class Socket;

class Observer {
public:
    virtual void react(Socket * pSocket)                                                                     = 0;
    virtual void react(Connection * pConnection, MessageType messageType, const std::vector<char> & message) = 0;
};

#endif // OBSERVER_H
