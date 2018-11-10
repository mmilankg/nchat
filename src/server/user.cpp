#include "user.h"
#include <iostream>

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
