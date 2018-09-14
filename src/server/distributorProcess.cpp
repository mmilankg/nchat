#include "distributorProcess.h"
#include <random>

DistributorProcess::DistributorProcess() : usersFileName("nchatUsers") {
  // Delete shared memory segments with the same names if they already exist.
  //boost::interprocess::shared_memory_object::remove("charMemory");
  boost::interprocess::shared_memory_object::remove("userVectorMemory");
  boost::interprocess::shared_memory_object::remove("messageQueueMemory");
  /*
   * Create new shared memory segments when the process starts.
   * Segment sizes are hard-coded to 65536 bytes.  It would be better
   * to define their sizes through constants (perhaps as enum members
   * of the class).
   */
  /*
  pCharSegment = new boost::interprocess::managed_shared_memory(
      boost::interprocess::create_only,
      "charMemory",
      65536
      );
      */
  pUserSegment = new boost::interprocess::managed_shared_memory(
      boost::interprocess::create_only,
      "userVectorMemory",
      65536
      );
  pMessageSegment = new boost::interprocess::managed_shared_memory(
      boost::interprocess::create_only,
      "messageQueueMemory",
      65536
      );
  // Initialize STL-compatible allocators.
  pUserAllocator = new UserAllocator(pUserSegment->get_segment_manager());
  pMessageAllocator = new MessageAllocator(pMessageSegment->get_segment_manager());
  // Construct the user vector and the message queue.
  pUserVector = pUserSegment->construct<UserVector>("userVector")(*pUserAllocator);
  pMessageQueue = pMessageSegment->construct<MessageQueue>("messageQueue")(*pMessageAllocator);
}

int DistributorProcess::run() {
  /* Open the file with user data and read it. */
  std::ifstream userFile(usersFileName.c_str());
  std::string line;
  // Parse the user file line by line.
  while (getline(userFile, line)) {
    std::istringstream ss(line);
    std::string field;
    /*
     * Parse the selected line field by field.  Fields are separated
     * by a colon.
     * 1. user id (int)
     * 2. user name (string)
     * 3. encrypted user password (string; first two characters are salt)
     * 4. list of contacts (integers separated by commas)
     */
    int userID;
    std::string username;
    std::string name;
    std::string password;
    std::string contacts;
    getline(ss, field, ':');
    userID = atoi(field.c_str());
    getline(ss, username, ':');
    getline(ss, name, ':');
    getline(ss, password, ':');
    getline(ss, contacts, ':');
    pUserVector->push_back(User(userID, username, name, password, offline));
  }

  /* DBG: open the memory segment for message queue allocation in order to read
   * message contents.  (Should be centralized so that it's done only once.) */
  boost::interprocess::managed_shared_memory messageQueueSegment(boost::interprocess::open_only, "messageQueueMemory");
  /* Read messages sent to the queue by client-oriented processes. */
  while (true) {
    // no action if the queue is empty
    if (pMessageQueue->size() == 0)
      continue;

    Message& message = pMessageQueue->front();

    /* Decode the message. */
    MessageType messageType = message.getType();
    pid_t messageSender = message.getSender();
    pid_t messageRecipient = message.getRecipient();
    char* msg = new char[message.getLength() + 1];
    message.read(msg);
    // Remove the message from the queue if the recepient matches the process ID.
    if (messageRecipient == pid) {
      // Release the memory allocated for the message contents;
      message.releaseContents();
      // Remove from the queue.
      pMessageQueue->pop_front();
    }

    // Process the message.
    switch (messageType) {
      case mCheckUsername :
	checkUsername(messageSender, msg);
	break;
      case mAddUser :
	addUser(msg);
	break;
      default :		    break;
    }
    delete []msg;
  }
  return 0;
}

void DistributorProcess::checkUsername(int clientProcessID, const std::string& username) const {
  /*
   * Iterate through the vector of users and check if a user with the same name already exists.
   * If the username is not taken, return a message to the client-dedicated process indicating
   * that the chosen username is acceptable ("OK").  If the name has already been taken, return
   * a message indicating that ("NOK").
   */
  boost::interprocess::vector<User, UserAllocator>::iterator userIterator;
  for (userIterator = pUserVector->begin(); userIterator != pUserVector->end(); userIterator++) {
    std::string registeredUsername = userIterator->getUsername();
    if (username == registeredUsername)
      break;
  }
  /*
   * Allow the username if the iterator reached the end of the vector without finding the match.
   * If the match was found, the loop is over before reaching the end of the vector.
   */
  if (userIterator == pUserVector->end())
    pMessageQueue->push_back(Message(pid, clientProcessID, mUsernameStatus, 2, "OK"));
  else
    pMessageQueue->push_back(Message(pid, clientProcessID, mUsernameStatus, 3, "NOK"));
}

/*
 * This function will receive an address to the location that contains all user data packed in
 * the following order:
 * 1. user full name
 * 2. username
 * 3. plain text user password
 * These fields will be separated by a null-byte for easy extraction.
 */
void DistributorProcess::addUser(const char* userData) const {
  /*
   * Extract the full name of the user, up to the first null-byte.  The string constructor
   * should automatically recognize the end of the name-sequence by finding the null-byte.
   */
  std::string name(userData);
  /* DBG: this should start extraction from the first character after the first null-byte! */
  std::string username(userData + name.length() + 1);
  /* DBG: start extraction from the first character after the second null-byte! */
  std::string password(userData + name.length() + username.length() + 2);
  /* DBG! */
  std::cout << name << " " << username << " " << password << std::endl;
  /*
   * The password is still in plain text.  It should be encrypted and salted before storing.
   *
   * DBG: Note: it seems to be a bad idea to send passwords unencrypted, so a suggested
   * improvement to this approach seems to be to send some key that a client can use to
   * encrypt the password before sending it to the server.  This would require changing
   * this part of the function.
   */
  /*
  char saltCandidates[64];
  for (int i = 0; i < 26; i++) {
    saltCandidates[i] = 'a' + i;
    saltCandidates[26 + i] = 'A' + i;
  }
  for (int i = 0; i < 10; i++)
    saltCandidates[52 + i] = '0' + i;
  saltCandidates[62] = '.';
  saltCandidates[63] = '/';
  // ... but perhaps cleaner to just define as a constant
  */
  const char* saltCandidates = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";
  char salt[2];
  /* DBG: perhaps better to have this as a class member and initialize in the constructor or
   * at the start of the run function in order to avoid reinitialization! */
  std::default_random_engine generator;
  std::uniform_int_distribution<int> distribution(0, 63);
  // Create salt from the first two randomly chosen characters from the set.
  salt[0] = saltCandidates[distribution(generator)];
  salt[1] = saltCandidates[distribution(generator)];
  char* encryptedPasswordContents = crypt(password.c_str(), salt);
  std::string encryptedPassword(encryptedPasswordContents);
  int userID = pUserVector->back().getUserID() + 1;
  pUserVector->push_back(User(userID, username, name, encryptedPassword, online));

  /*
   * Add the user to the file listing all users and their contacts.
   * This should be done with the locking mechanism so that the
   * process has an exclusive access to the file while the write
   * operation is ongoing.  This is left for later implementation.
   */
  /*
  std::ifstream userFile(rUsersFileName.c_str());
  std::string line;
  // Parse the user file line by line.
  while (getline(userFile, line)) {
    std::istringstream ss(line);
    std::string field;
    *
     * Parse the selected line field by field.  Fields are separated
     * by a colon.
     * 1. user id (int)
     * 2. user name (string)
     * 3. encrypted user password (string; first two characters are salt)
     * 4. list of contacts (integers separated by commas)
     *
    int userID;
    std::string username;
    std::string name;
    std::string password;
    std::string contacts;
    getline(ss, field, ':');
    userID = atoi(field.c_str());
    getline(ss, username, ':');
    getline(ss, name, ':');
    getline(ss, password, ':');
    getline(ss, contacts, ':');
    User* pUser = new User(userID, username, name, password, offline);
    users.push_back(pUser);
  }
  */
}
