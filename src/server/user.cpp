#include "user.h"
#include <iostream>

std::ostream & operator<<(std::ostream & os, const User & user)
{
    os << user.userID << ":" << user.username << ":" << user.name << ":" << user.encryptedPassword << ":";
    if (user.contactIDs.size() > 0) {
        auto contactIterator = user.contactIDs.begin();
        os << *contactIterator++;
        for (; contactIterator != user.contactIDs.end(); contactIterator++) os << "," << *contactIterator;
    }
    os << ":" << std::endl;
    return os;
}
