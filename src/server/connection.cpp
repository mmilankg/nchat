#include "connection.h"
#include "trace.h"
#include <cassert>
#include <vector>

extern int verbosityLevel;

void Connection::receive()
{
    // Set up the buffer for receiving messages.
    std::vector<char> message;
    int               messageLength;
    MessageType       messageType;
    pClientSocket->recv(messageLength);
    assert(messageLength > 0);
    pClientSocket->recv(messageType);
    pClientSocket->recv(message, messageLength - sizeof(messageLength) - sizeof(messageType));
    TRACE(verbosityLevel, "message received")
    pObserver->react(this, messageType, message);
}
