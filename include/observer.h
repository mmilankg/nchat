#ifndef OBSERVER_H
#define OBSERVER_H

class Observer {
public:
    virtual void react(void * pObject) = 0;
};

#endif // OBSERVER_H
