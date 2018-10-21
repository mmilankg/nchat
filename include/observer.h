#ifndef OBSERVER_H
#define OBSERVER_H

#include "socket.h"
#include <vector>

class Observer {
public:
    virtual void react(Socket * pObject)                  = 0;
    virtual void react(const std::vector<char> & message) = 0;
};

#endif // OBSERVER_H
