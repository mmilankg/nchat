#include "message.h"
#include "user.h"
#include <iostream>

void User::transmit(Connection * conn, MessageType messageType) const
{
    int messageContentLength = sizeof(userID) + username.length() + 1 + name.length() + 1 + sizeof(status);
    std::vector<char> messageContent;
    messageContent.resize(messageContentLength);
    int nBytesProcessed = 0;
    std::memcpy(messageContent.data() + nBytesProcessed, &userID, sizeof(userID));
    nBytesProcessed += sizeof(userID);
    std::memcpy(messageContent.data() + nBytesProcessed, username.c_str(), username.length());
    nBytesProcessed += username.length() + 1;
    std::memcpy(messageContent.data() + nBytesProcessed, name.c_str(), name.length());
    nBytesProcessed += name.length() + 1;
    std::memcpy(messageContent.data() + nBytesProcessed, &status, sizeof(status));
    conn->transmit(messageType, messageContent);
}

std::ostream & operator<<(std::ostream & os, const User & user)
{
    os << user.userID << ":" << user.username << ":" << user.name << ":" << user.encryptedPassword << ":";
    // Write established contacts.
    if (user.contactIDs.size() > 0) {
        auto contactIterator = user.contactIDs.begin();
        os << *contactIterator++;
        for (; contactIterator != user.contactIDs.end(); contactIterator++) os << "," << *contactIterator;
    }
    os << ":";
    // Write sent contact requests.
    if (user.sentContactRequestIDs.size() > 0) {
        auto contactIterator = user.sentContactRequestIDs.begin();
        os << *contactIterator++;
        for (; contactIterator != user.sentContactRequestIDs.end(); contactIterator++) os << "," << *contactIterator;
    }
    os << ":";
    // Write received contact requests.
    if (user.receivedContactRequestIDs.size() > 0) {
        auto contactIterator = user.receivedContactRequestIDs.begin();
        os << *contactIterator++;
        for (; contactIterator != user.receivedContactRequestIDs.end(); contactIterator++)
            os << "," << *contactIterator;
    }
    os << ":" << std::endl;
    return os;
}
