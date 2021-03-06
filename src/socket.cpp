#include "message.h"
#include "socket.h"
#include <unistd.h>
#include <cassert>
#include <cstring>
#include <string>
#include <sstream>

/*
 * Socket initialisation is based on example from man getaddrinfo.  The constructor does most of the work in setting up
 * the connection.  For the server, it would create a socket, bind it to an interface and prepare for listenning.  For
 * the client, it will create a socket and connect it to the server.  The server socket is activated by passing an
 * emptry string as the hostName.
 */
Socket::Socket(const std::string & hostName, int port, int bs) : bufSize(bs)
{
    int             ret; // placeholder for return values of various system functions
    struct addrinfo hints;
    std::memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family   = AF_UNSPEC;   // for allowing IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // socket for TCP connection
    if (hostName == "")
        hints.ai_flags = AI_PASSIVE; // for wildcard IP address
    else
        hints.ai_flags = 0;
    hints.ai_protocol = 0; // any protocol

    // resulting struct addrinfo instance
    struct addrinfo * pResult;
    const char *      pHostName = hostName.c_str();
    if (hostName == "") pHostName = NULL;
    std::string sPort = std::to_string(port);
    ret               = getaddrinfo(pHostName, sPort.c_str(), &hints, &pResult);
    assert(ret == 0);
    for (struct addrinfo * pR = pResult; pR != NULL; pR = pR->ai_next) {
        sfd = socket(pR->ai_family, pR->ai_socktype, pR->ai_protocol);
        assert(sfd != -1);
        if (hostName == "") {
            ret = bind(sfd, pR->ai_addr, pR->ai_addrlen);
            assert(ret == 0);
            std::memcpy(&networkAddress, pR->ai_addr, sizeof(struct sockaddr));
            break;
        }
        else {
            ret = connect(sfd, pR->ai_addr, pR->ai_addrlen);
            assert(ret == 0);
            std::memcpy(&networkAddress, pR->ai_addr, sizeof(struct sockaddr));
            break;
        }
    }
    // Release the resulting struct.
    freeaddrinfo(pResult);
    if (hostName == "") {
        // maximum number of pending connections (from man listen)
        int backlog = 100;
        ret         = listen(sfd, backlog);
        assert(ret == 0);
    }

    // Allocate buffer memory.
    buffer = new char[bufSize];
}

Socket Socket::acceptConnection()
{
    struct sockaddr clientAddress;
    socklen_t       clientAddressSize = sizeof(clientAddress);
    int             cfd               = accept(sfd, &clientAddress, &clientAddressSize);
    assert(cfd != -1);
    Socket communicationSocket;
    communicationSocket.setSfd(cfd);
    communicationSocket.bufSize = bufSize;
    communicationSocket.allocateBuffer();
    communicationSocket.networkAddress = clientAddress;
    return communicationSocket;
}

// Send a set of strings in one packed message.
void Socket::send(MessageType messageType, const std::string & text)
{
    // Add 1 for terminating null-byte for each string.
    int messageLength = sizeof(messageLength) + sizeof(messageType) + text.length() + 1;
    // Extend the socket-associated buffer if the message length exceeds its size.
    if (messageLength > bufSize) {
        bufSize = messageLength;
        delete[] buffer;
        buffer = new char[messageLength]();
    }
    /* Pack the buffer in the following order:
     * 1. messageLength
     * 2. messageType
     * 3. text finishing with null-byte
     */
    char * buf = buffer;
    std::memcpy(buf, &messageLength, sizeof(messageLength));
    buf += sizeof(messageLength);
    std::memcpy(buf, &messageType, sizeof(messageType));
    buf += sizeof(messageType);
    std::memcpy(buf, text.c_str(), text.length());
    buf += text.length();
    std::memset(buf, 0, 1);
    write(sfd, buffer, messageLength);
}

void Socket::send(MessageType messageType, const std::vector<std::string> & parts)
{
    int                                      messageLength = sizeof(messageLength) + sizeof(messageType);
    std::vector<std::string>::const_iterator partsIterator;
    for (partsIterator = parts.begin(); partsIterator != parts.end(); partsIterator++)
        // Add 1 for terminating null-byte for each string.
        messageLength += partsIterator->length() + 1;
    // Extend the socket-associated buffer if the message length exceeds
    // its size.
    if (messageLength > bufSize) {
        bufSize = messageLength;
        delete[] buffer;
        buffer = new char[messageLength]();
    }
    /* Pack the buffer in the following order:
     * 1. messageLength
     * 2. messageType
     * 3. string parts, each finishing with null-byte.
     */
    char * buf = buffer;
    std::memcpy(buf, &messageLength, sizeof(messageLength));
    buf += sizeof(messageLength);
    std::memcpy(buf, &messageType, sizeof(messageType));
    buf += sizeof(messageType);
    for (partsIterator = parts.begin(); partsIterator != parts.end(); partsIterator++) {
        std::memcpy(buf, partsIterator->c_str(), partsIterator->length());
        buf += partsIterator->length();
        std::memset(buf, 0, 1);
        buf++;
    }
    write(sfd, buffer, messageLength);
}

void Socket::recv(std::vector<char> & buffer, int count) const
{
    buffer.resize(count);
    int nRead = read(sfd, buffer.data(), count);
    assert(nRead == count);
}

void Socket::recv(std::string & text) const
{
    /* DBG: check the appropriate way to complete reading of a long message! */
    /*
    int numBytesRead = 0;
    while ((numBytesRead = read(sfd, buffer, bufSize)) > 0)
      text += buffer;
      */
    int numBytesRead = read(sfd, buffer, bufSize);
    if (numBytesRead < bufSize) buffer[numBytesRead] = 0;
    text = buffer;
}

// Receive a set of strings in one packed message.
void Socket::recv(MessageType & messageType, std::vector<std::string> & parts)
{
    int messageLength = 0;
    read(sfd, &messageLength, sizeof(messageLength));
    read(sfd, &messageType, sizeof(messageType));
    int remainingLength = messageLength - sizeof(messageLength) - sizeof(messageType);
    // Extend the socket-associated buffer if the remaining length exceeds
    // its size.
    if (remainingLength > bufSize) {
        bufSize = remainingLength;
        delete[] buffer;
        buffer = new char[remainingLength]();
    }
    read(sfd, buffer, remainingLength);
    char * buf = buffer;
    // Process each string terminated with null-byte until the
    // remainingLength drops to 0.
    while (remainingLength > 0) {
        parts.push_back(buf);
        remainingLength -= parts.back().length();
        buf += parts.back().length();
        // Add 1 for null-byte.
        buf++;
        remainingLength--;
    }
}
