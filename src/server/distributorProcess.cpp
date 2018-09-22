#include "distributorProcess.h"
#include <chrono>

DistributorProcess::DistributorProcess() :
  usersFileName("nchatUsers"),
  // Initialize the salt generator using the current time.
  saltGenerator(std::chrono::system_clock::now().time_since_epoch().count()),
  // Initialize the random distribution object.
  saltDistribution(std::uniform_int_distribution<>(0, 63))
{
  // Delete shared memory segments with the same names if they already exist.
  boost::interprocess::shared_memory_object::remove("userVectorMemory");
  boost::interprocess::shared_memory_object::remove("messageQueueMemory");
  /*
   * Create new shared memory segments when the process starts.
   * Segment sizes are hard-coded to 65536 bytes.  It would be better
   * to define their sizes through constants (perhaps as enum members
   * of the class).
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

  // Open the file for reading and writing in the append mode.
  usersFile.open(usersFileName.c_str(), std::ios::in | std::ios::out | std::ios::app);
}

int DistributorProcess::run() {
  // Parse the user file line by line.
  std::string line;
  while (getline(usersFile, line)) {
    std::istringstream ss(line);
    std::string field;
    /*
     * Parse the selected line field by field.  Fields are separated
     * by a colon.
     * 1. user id (int)
     * 2. name (string)
     * 3. username (string)
     * 4. encrypted user password (string; first two characters are salt)
     * 5. list of contacts (integers separated by commas)
     */
    pid_t processID = 0;
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
    pUserVector->push_back(User(processID, userID, username, name, password, offline, contacts));
  }
  /*
   * Clear the error state flag after reading in order to be able to write to
   * the file in other functions:
   *
   * https://stackoverflow.com/questions/32435991/how-to-read-and-write-in-file-with-fstream-simultaneously-in-c/32437476#32437476
   */
  usersFile.clear();

  /* DBG: open the memory segment for message queue allocation in order to read
   * message contents.  (Should be centralized so that it's done only once.) */
  boost::interprocess::managed_shared_memory messageQueueSegment(boost::interprocess::open_only, "messageQueueMemory");
  /* Read messages sent to the queue by client-oriented processes. */
  while (true) {
    // no action if the queue is empty
    if (pMessageQueue->size() == 0)
      continue;

    Message& message = pMessageQueue->front();

    pid_t messageRecipient = message.getRecipient();
    // Remove the message from the queue if the recepient matches the process ID.
    if (messageRecipient == pid) {
      /* Decode the message. */
      MessageType messageType = message.getType();
      pid_t messageSender = message.getSender();
      char* msg = new char[message.getLength() + 1];
      message.read(msg);
      // Release the memory allocated for the message contents;
      message.releaseContents();
      // Remove from the queue.
      pMessageQueue->pop_front();

      // Process the message.
      switch (messageType) {
	case mCheckUsername :
	  checkUsername(messageSender, msg);
	  break;
	case mAddUser :
	  addUser(messageSender, msg);
	  break;
	case mCheckUser :
	  checkUser(messageSender, msg);
	  break;
	case mLogoutUser :
	  logoutUser(messageSender);
	  break;
      }
      delete []msg;
    }
  }
  return 0;
}

void DistributorProcess::checkUsername(pid_t clientProcessID, const std::string& username) const {
  /*
   * Iterate through the vector of users and check if a user with the same name already exists.
   * If the username is not taken, return a message to the client-dedicated process indicating
   * that the chosen username is acceptable ("OK").  If the name has already been taken, return
   * a message indicating that ("NOK").
   */
  boost::interprocess::vector<User, UserAllocator>::iterator userIterator;
  for (userIterator = pUserVector->begin(); userIterator != pUserVector->end(); userIterator++) {
    std::string storedUsername = userIterator->getUsername();
    if (username == storedUsername)
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

void DistributorProcess::checkUser(pid_t clientProcessID, const char* buffer) const {
  // Unpack the username and password from the message.
  std::string username(buffer);
  std::string password(buffer + username.length() + 1);
  boost::interprocess::vector<User, UserAllocator>::iterator userIterator;
  for (userIterator = pUserVector->begin(); userIterator != pUserVector->end(); userIterator++) {
    std::string storedUsername = userIterator->getUsername();
    // When the user is found:
    if (username == storedUsername) {
      // Check the password.  Take the stored password from the database and extract the
      // first two characters as salt.
      std::string storedPassword = userIterator->getPassword();
      char salt[2];
      std::memcpy(salt, storedPassword.c_str(), 2);
      char* encryptedPasswordContents = crypt(password.c_str(), salt);
      std::string encryptedPassword(encryptedPasswordContents);
      if (encryptedPassword == storedPassword) {
	pMessageQueue->push_back(Message(pid, clientProcessID, mCheckUser, 2, "OK"));
	// Mark the user's status in the list of all users as online.
	userIterator->setStatus(online);
      }
      else
	pMessageQueue->push_back(Message(pid, clientProcessID, mCheckUser, 3, "NOK"));
      // Break iterations if the user has been found.
      break;
    }
  }

  /*
   * If the loop finishes without finding the user (iterator == vector.end()), the
   * username that was sent is wrong as no such user has been found in the system.
   * Return "UNK" message to the client-dedicated process.
   */
  if (userIterator == pUserVector->end())
    pMessageQueue->push_back(Message(pid, clientProcessID, mCheckUser, 3, "UNK"));
}

/*
 * This function will receive an address to the location that contains all user data packed in
 * the following order:
 * 1. user full name
 * 2. username
 * 3. plain text user password
 * These fields will be separated by a null-byte for easy extraction.
 */
void DistributorProcess::addUser(pid_t clientProcessID, const char* userData) {
  /*
   * Extract the full name of the user, up to the first null-byte.  The string constructor
   * should automatically recognize the end of the name-sequence by finding the null-byte.
   */
  std::string name(userData);
  /* DBG: this should start extraction from the first character after the first null-byte! */
  std::string username(userData + name.length() + 1);
  /* DBG: start extraction from the first character after the second null-byte! */
  std::string password(userData + name.length() + username.length() + 2);
  /*
   * The password is still in plain text.  It should be encrypted and prepended with salt
   * before storing.
   *
   * DBG: Note: it seems to be a bad idea to send passwords unencrypted, so a suggested
   * improvement to this approach seems to be to send some key that a client can use to
   * encrypt the password before sending it to the server.  This would require changing
   * this part of the function.
   */
  const char* saltCandidates = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";
  char salt[2];
  // Create salt from the first two randomly chosen characters from the set.
  salt[0] = saltCandidates[saltDistribution(saltGenerator)];
  salt[1] = saltCandidates[saltDistribution(saltGenerator)];
  char* encryptedPasswordContents = crypt(password.c_str(), salt);
  std::string encryptedPassword(encryptedPasswordContents);
  int userID = 0;
  if (pUserVector->size() > 0)
    userID = pUserVector->back().getUserID() + 1;
  // empty string for contacts as the last argument
  pUserVector->push_back(User(clientProcessID, userID, username, name, encryptedPassword, online, ""));

  /*
   * Add the user to the file listing all users and their contacts.
   * This should be done with the locking mechanism so that the
   * process has an exclusive access to the file while the write
   * operation is ongoing.  This is left for later implementation.
   */
  /*
   * Pack user information into a single line with the following format:
   * userID:name:username:encryptedPassword:
   * These are separated by colons.  The last colon is the end of the line
   * for a new user.  For an existing user, it will be followed by the list
   * of contacts: comma-separated userIDs.
   */
  std::string line = std::to_string(userID) + ":";
  line += username + ":";
  line += name + ":";
  line += encryptedPassword + ":";
  usersFile << line << std::endl;
}

void DistributorProcess::logoutUser(pid_t clientProcessID) {
  /*
   * Set the process ID for this user's client-dedicated process as 0, set the user status as offline,
   * and inform contacts to update their status flags for the user.
   */
  boost::interprocess::vector<User, UserAllocator>::iterator userIterator;
  for (userIterator = pUserVector->begin(); userIterator != pUserVector->end(); userIterator++) {
    pid_t iteratedUsersProcessID = userIterator->getProcessID();
    if (clientProcessID == iteratedUsersProcessID) {
      // The user has been found, so update the corresponding fields in the entry.
      userIterator->setProcessID(0);
      userIterator->setStatus(offline);
      // Iterate through the list of contacts and inform other users that this one has logged out.
      const std::vector<int>& rContactIDs = userIterator->getContactIDs();
      std::vector<int>::const_iterator contactsIterator;
      for (contactsIterator = rContactIDs.begin(); contactsIterator != rContactIDs.end(); contactsIterator++) {
	/* DBG: This is going to be extremely inefficient as it involves double loop!  There should be a better
	 * way to store user objects so that the process ID is retrieved automatically with user ID. */
	boost::interprocess::vector<User, UserAllocator>::iterator innerUserIterator;
	for (innerUserIterator = pUserVector->begin(); innerUserIterator != pUserVector->end(); innerUserIterator++) {
	  int contactID = innerUserIterator->getUserID();
	  if (contactID == *contactsIterator) {
	    /* If the contact has been found, send it a message about the user being logged out. */
	    // Get the process ID.
	    pid_t contactProcessID = innerUserIterator->getProcessID();
	    pMessageQueue->push_back(Message(pid, contactProcessID, mLogoutUser, 0, ""));
	    break;
	  }
	}
      }
      break;
    }
  }
}
