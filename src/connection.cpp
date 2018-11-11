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
    pSocket->recv(messageLength);
    assert(messageLength > 0);
    pSocket->recv(messageType);
    pSocket->recv(message, messageLength - sizeof(messageLength) - sizeof(messageType));
    TRACE(verbosityLevel, "message received")
    pObserver->react(this, messageType, message);
}

void Connection::transmit(MessageType messageType, const std::vector<char> & messageContent)
{
    int messageLength = sizeof(messageLength) + sizeof(messageType) + messageContent.size();
    pSocket->send(messageLength);
    pSocket->send(messageType);
    pSocket->send(messageContent);
}

void Connection::transmit(MessageType messageType, const std::string & messageContent)
{
    pSocket->send(messageType, messageContent);
}

void Connection::transmit(MessageType messageType, int messageContent)
{
    int messageLength = sizeof(messageLength) + sizeof(messageType) + sizeof(messageContent);
    pSocket->send(messageLength);
    pSocket->send(messageType);
    pSocket->send(messageContent);
}
