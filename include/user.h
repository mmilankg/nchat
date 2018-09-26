#ifndef USER_H
#define USER_H

#include "socket.h"
#include <sstream>
#include <string>
#include <vector>

// enumerated list defining user status
enum Status {
  online,
  busy,
  offline
};

// user properties found in the file listing all users
class User {
  int userID;
  std::string username;
  std::string name;
  std::string encryptedPassword;
  // address of the Socket object through which the user is connected
  Socket* clientSocket;
  Status status;
  std::vector<int> contactIDs;

  public:
  User() : userID(0), clientSocket(0), status(offline) { }
  User(int uid, const std::string& uname, const std::string& nm, const std::string& pwd, Socket* pSocket, Status st, const std::string& contacts) :
    userID(uid), username(uname), name(nm), encryptedPassword(pwd), clientSocket(pSocket), status(st)
  {
    std::istringstream contactStream(contacts);
    std::string field;
    while (getline(contactStream, field, ',')) {
      int contactID = atoi(field.c_str());
      contactIDs.push_back(contactID);
    }
  }
  User(const User& u) :
    userID(u.userID), username(u.username), name(u.name),
    encryptedPassword(u.encryptedPassword),
    clientSocket(u.clientSocket), status(u.status),
    contactIDs(u.contactIDs)
  { }
  virtual ~User() { }

  int getUserID() const { return userID; }
  void setUserID(int uid) { userID = uid; }
  const std::string& getUsername() const { return username; }
  void setUsername(const std::string& uname) { username = uname; }
  const std::string& getName() const { return name; }
  void setName(const std::string& nm) { name = nm; }
  const std::string& getPassword() const { return encryptedPassword; }
  void setPassword(const std::string& pwd) { encryptedPassword = pwd; }
  Socket* getClientSocket() { return clientSocket; }
  void setClientSocket(Socket* pSocket) { clientSocket = pSocket; }
  Status getStatus() const { return status; }
  void setStatus(Status st) { status = st; }
  const std::vector<int>& getContactIDs() const { return contactIDs; }
  void setContactIDs(const std::vector<int>& cids) { contactIDs = cids; }
};

// stripped down class for representing a user as a contact of another user
/* DBG: This doesn't seem like an optimal solution as it reuses many functions
 * from the User class.  A better approach would be to have one as a base class
 * and have the other inherit from it. */
class Contact {
  int userID;
  std::string username;
  std::string name;
  Status status;

  public:
  Contact() : userID(0) { }
  Contact(const Contact& c) :
    userID(c.userID), username(c.username), name(c.name), status(c.status)
  { }
  virtual ~Contact() { }

  int getUserID() const { return userID; }
  void setUserID(int uid) { userID = uid; }
  const std::string& getUsername() const { return username; }
  void setUsername(const std::string& uname) { username = uname; }
  const std::string& getName() const { return name; }
  void setName(const std::string& nm) { name = nm; }
  Status getStatus() const { return status; }
  void setStatus(Status st) { status = st; }
};

#endif	// USER_H
