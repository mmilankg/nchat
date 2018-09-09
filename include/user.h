#ifndef USER_H
#define USER_H

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
  Status status;
  std::vector<int> contactIDs;
  public:
  User() : userID(0), status(offline) { }
  User(int uid, const std::string& uname, const std::string& nm, const std::string& pwd, Status st) :
    userID(uid), username(uname), name(nm), encryptedPassword(pwd), status(st)
  { }
  User(const User& u) : userID(u.userID), username(u.username), encryptedPassword(u.encryptedPassword), status(u.status) { }
  virtual ~User() { }
  virtual int getUserID() const { return userID; }
  virtual void setUserID(int uid) { userID = uid; }
  virtual const std::string& getUsername() const { return username; }
  virtual void setUsername(const std::string& uname) { username = uname; }
  virtual const std::string& getName() const { return name; }
  virtual void setName(const std::string& nm) { name = nm; }
  virtual const std::string& getPassword() const { return encryptedPassword; }
  virtual void setPassword(const std::string& pwd) { encryptedPassword = pwd; }
  virtual Status getStatus() const { return status; }
  virtual void setStatus(Status st) { status = st; }
};

class Contact : public User {
  /*
   * ID for the client dedicated process if
   * the user is logged in; 0 if not logged in.
   */
  int processID;
  public:
  Contact() : processID(0) { }
  Contact(const Contact& c) : User(c), processID(c.processID) { }
  ~Contact() { }
  // sending a text message to the user
  void text(const std::string& msg) { /* implement later */ }
  // call the user
  void call() { /* implement later */ }
};

#endif	// USER_H
