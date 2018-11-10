#ifndef USER_H
#define USER_H

#include "connection.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// enumerated list defining user status
enum Status { online, busy, offline };

// user properties found in the file listing all users
class User {
    int         userID;
    std::string username;
    std::string name;
    std::string encryptedPassword;
    // address of the Connection object through which the user is connected
    Connection *     connection;
    Status           status;
    std::vector<int> contactIDs;
    std::vector<int> sentContactRequestIDs;
    std::vector<int> receivedContactRequestIDs;

public:
    User() : userID(0), connection(0), status(offline) {}
    User(int                 uid,
         const std::string & uname,
         const std::string & nm,
         const std::string & pwd,
         Connection *        conn,
         Status              st,
         const std::string & contacts,
         const std::string & sentContactRequests,
         const std::string & receivedContactRequests) :
        userID(uid),
        username(uname),
        name(nm),
        encryptedPassword(pwd),
        connection(conn),
        status(st)
    {
        std::istringstream contactStream(contacts);
        std::istringstream sentContactRequestStream(sentContactRequests);
        std::istringstream receivedContactRequestStream(receivedContactRequests);
        std::string        field;
        while (getline(contactStream, field, ',')) {
            int contactID = atoi(field.c_str());
            contactIDs.push_back(contactID);
        }
        while (getline(sentContactRequestStream, field, ',')) {
            int sentContactRequestID = atoi(field.c_str());
            sentContactRequestIDs.push_back(sentContactRequestID);
        }
        while (getline(receivedContactRequestStream, field, ',')) {
            int receivedContactRequestID = atoi(field.c_str());
            receivedContactRequestIDs.push_back(receivedContactRequestID);
        }
    }
    User(const User & u) :
        userID(u.userID),
        username(u.username),
        name(u.name),
        encryptedPassword(u.encryptedPassword),
        connection(u.connection),
        status(u.status),
        contactIDs(u.contactIDs),
        sentContactRequestIDs(u.sentContactRequestIDs),
        receivedContactRequestIDs(u.receivedContactRequestIDs)
    {
    }
    virtual ~User() {}

    int                      getUserID() const { return userID; }
    void                     setUserID(int uid) { userID = uid; }
    const std::string &      getUsername() const { return username; }
    void                     setUsername(const std::string & uname) { username = uname; }
    const std::string &      getName() const { return name; }
    void                     setName(const std::string & nm) { name = nm; }
    const std::string &      getPassword() const { return encryptedPassword; }
    void                     setPassword(const std::string & pwd) { encryptedPassword = pwd; }
    Connection *             getConnection() { return connection; }
    void                     setConnection(Connection * conn) { connection = conn; }
    Status                   getStatus() const { return status; }
    void                     setStatus(Status st) { status = st; }
    const std::vector<int> & getContactIDs() const { return contactIDs; }
    void                     setContactIDs(const std::vector<int> & cids) { contactIDs = cids; }
    std::vector<int> &       getSentContactRequestIDs() { return sentContactRequestIDs; }
    void                     setSentContactRequestIDs(const std::vector<int> & crids) { sentContactRequestIDs = crids; }
    std::vector<int> &       getReceivedContactRequestIDs() { return receivedContactRequestIDs; }
    void setReceivedContactRequestIDs(const std::vector<int> & crids) { receivedContactRequestIDs = crids; }
    void addContact(int contactID) { contactIDs.push_back(contactID); }
    void transmit(Connection * conn, MessageType messageType) const;

    friend std::ostream & operator<<(std::ostream & os, const User & user);
};

// stripped down class for representing a user as a contact of another user
/* DBG: This doesn't seem like an optimal solution as it reuses many functions
 * from the User class.  A better approach would be to have one as a base class
 * and have the other inherit from it. */
class Contact {
    int         userID;
    std::string username;
    std::string name;
    Status      status;

public:
    Contact() : userID(0) {}
    Contact(const Contact & c) : userID(c.userID), username(c.username), name(c.name), status(c.status) {}
    virtual ~Contact() {}

    int                 getUserID() const { return userID; }
    void                setUserID(int uid) { userID = uid; }
    const std::string & getUsername() const { return username; }
    void                setUsername(const std::string & uname) { username = uname; }
    const std::string & getName() const { return name; }
    void                setName(const std::string & nm) { name = nm; }
    Status              getStatus() const { return status; }
    void                setStatus(Status st) { status = st; }
};

#endif // USER_H
