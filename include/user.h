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
  pid_t processID;		  // process ID for the process at which the user is connected
  int userID;
  std::string username;
  std::string name;
  std::string encryptedPassword;
  Status status;
  std::vector<int> contactIDs;
  public:
  User() : processID(0), userID(0), status(offline) { }
  User(pid_t pid, int uid, const std::string& uname, const std::string& nm, const std::string& pwd, Status st, const std::string& contacts) :
    processID(pid), userID(uid), username(uname), name(nm), encryptedPassword(pwd), status(st)
  {
    std::istringstream contactStream(contacts);
    std::string field;
    while (getline(contactStream, field, ','));
    int contactID = atoi(field.c_str());
    contactIDs.push_back(contactID);
  }
  User(const User& u) :
    processID(u.processID), userID(u.userID),
    username(u.username), encryptedPassword(u.encryptedPassword),
    status(u.status), contactIDs(u.contactIDs)
  { }
  virtual ~User() { }
  virtual int getProcessID() const { return processID; }
  virtual void setProcessID(int pid) { processID = pid; }
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
  virtual const std::vector<int>& getContactIDs() const { return contactIDs; }
  virtual void setContactIDs(const std::vector<int>& cids) { contactIDs = cids; }
};

#endif	// USER_H
