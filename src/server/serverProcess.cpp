#include "message.h"
#include "serverProcess.h"
#include <signal.h>
#include <chrono>

ServerProcess::ServerProcess() :
  usersFileName("nchatUsers"),
  // Open the file for reading and writing in the append mode.
  usersFile(usersFileName.c_str(), std::ios::in | std::ios::out | std::ios::app),
  // passing an empty string sets up the listening socket
  listeningSocket(""),
  // Initialize the salt generator using the current time.
  saltGenerator(std::chrono::system_clock::now().time_since_epoch().count()),
  // Initialize the random distribution object.
  saltDistribution(std::uniform_int_distribution<>(0, 63))
{
  // Open the file for reading and writing in the append mode.
  //usersFile.open(usersFileName.c_str(), std::ios::in | std::ios::out | std::ios::app);
  // Parse the user file line by line.
  std::string line;
  while (getline(usersFile, line)) {
    std::istringstream ss(line);
    std::string field;
    /*
     * Parse the selected line field by field.  Fields are separated
     * by a colon.
     * 1. user id (int)
     * 2. username (string)
     * 3. real name (string)
     * 4. encrypted user password (string; first two characters are salt)
     * 5. list of contacts (integers separated by commas)
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
    users.push_back(User(userID, username, name, password, 0, offline, contacts));
  }
  /*
   * Clear the error state flag after reading in order to be able to write to
   * the file in other functions:
   *
   * https://stackoverflow.com/questions/32435991/how-to-read-and-write-in-file-with-fstream-simultaneously-in-c/32437476#32437476
   */
  usersFile.clear();
}

int ServerProcess::run() {
  /*
   * Ignore the SIGPIPE signal, but later add the functionality to clean
   * up the client if the connection is broken.
   */
  struct sigaction signalAction;
  signalAction.sa_handler = SIG_IGN;
  //sigaction(SIGPIPE, &signalAction, 0);

  /*
   * Prepare for the select() call so that the listening at a socket
   * doesn't completely block the execution.
   */
  fd_set socketDescriptors;
  FD_ZERO(&socketDescriptors);
  int nSockets = listeningSocket.getSfd() + clientSockets.size();
  FD_SET(nSockets, &socketDescriptors);

  // set up buffer for receiving messages
  /* DBG: Perhaps the buffer address should be a class member.  And it
   * would be good to set the buffer size as a constant element. */
  char* buffer = new char[1024]();
  while (true) {
    /*
     * Start the select() function, but only for reading when sockets
     * are ready.  Set the last value to 0 in order to listen
     * indefinitely.
     */
    select(nSockets + 1, &socketDescriptors, 0, 0, 0);

    // when listening socket is ready to accept
    if (FD_ISSET(listeningSocket.getSfd(), &socketDescriptors)) {
      /* DBG: a better way would be for the acceptConnection to return a
       * pointer directly.  That would avoid having to call the
       * copy-constructor. */
      Socket* pSocket = new Socket(listeningSocket.acceptConnection());
      clientSockets.push_back(pSocket);
      /* Update the number of sockets and the list of descriptors for
       * the select() call. */
      nSockets++;
      FD_SET(nSockets, &socketDescriptors);
    }
    // for messages on client sockets
    else {
      // Loop through all client sockets to see if they are selected.
      std::vector<Socket*>::iterator sockIt;
      for (sockIt = clientSockets.begin(); sockIt != clientSockets.end(); sockIt++) {
	Socket* clientSocket = *sockIt;
	int clientSocketFD = clientSocket->getSfd();
	if (FD_ISSET(clientSocketFD, &socketDescriptors)) {
	  int messageLength;
	  MessageType messageType;
	  clientSocket->recv(messageLength);
	  clientSocket->recv(messageType);
	  clientSocket->recv(buffer);
	  int contentLength = messageLength - sizeof(messageLength) - sizeof(messageType);

	  // Process the message.
	  switch (messageType) {
	    case mSignup :
	      {
		std::vector<std::string> userDetails;
		bufferToStrings(buffer, contentLength, userDetails);
		signup(clientSocket, userDetails);
		break;
	      }
	    case mLogin :
	      {
		std::vector<std::string> userDetails;
		bufferToStrings(buffer, contentLength, userDetails);
		login(clientSocket, userDetails);
		break;
	      }
	    case mLogout :
	      {
		std::string username = buffer;
		logout(clientSocket, username);
		break;
	      }
	  }
	}
      }
    }
  }

  return 0;
}

void ServerProcess::signup(Socket* clientSocket, const std::vector<std::string>& userDetails) {
  // Unpack the name, username and password from the message.
  std::string name = userDetails[0];
  std::string username = userDetails[1];
  std::string password = userDetails[2];

  int response;	  // response to the client
  if (checkUsername(username) == 0) {
    response = 0;
    addUser(clientSocket, userDetails);
  }
  else
    response = 1;

  // Return the response to the client.
  MessageType messageType = mLogin;
  int messageLength = sizeof(messageLength) + sizeof(messageType) + sizeof(response);
  clientSocket->send(messageLength);
  clientSocket->send(messageType);
  clientSocket->send(response);
}

void ServerProcess::login(Socket* clientSocket, const std::vector<std::string>& userDetails) {
  // Unpack the username and password from the message.
  std::string username = userDetails[0];
  std::string password = userDetails[1];
  std::vector<User>::iterator userIt;
  int response;
  for (userIt = users.begin(); userIt != users.end(); userIt++) {
    std::string storedUsername = userIt->getUsername();
    // When the user is found:
    if (username == storedUsername) {
      // Check the password.  Take the stored password from the database and extract the
      // first two characters as salt.
      std::string storedPassword = userIt->getPassword();
      char salt[2];
      std::memcpy(salt, storedPassword.c_str(), 2);
      char* encryptedPasswordContents = crypt(password.c_str(), salt);
      std::string encryptedPassword(encryptedPasswordContents);
      if (encryptedPassword == storedPassword)
	response = 0;
      else
	response = 1;
      // Break iterations if the user has been found.
      break;
    }
  }

  /*
   * If the loop finishes without finding the user (iterator == vector.end()), the
   * username that was sent is wrong as no such user has been found in the system.
   * Return response 2 to the client.
   */
  if (userIt == users.end())
    response = 2;

  MessageType messageType = mLogin;
  int messageLength = sizeof(messageLength) + sizeof(messageType) + sizeof(response);
  clientSocket->send(messageLength);
  clientSocket->send(messageType);
  clientSocket->send(response);

  /* If the user is logged in, set its status to online and send it the
   * list of contacts. */
  if (response == 0) {
    // Mark the user's status in the list of all users as online.
    userIt->setStatus(online);

    // Send the list of contacts to the user.
    const std::vector<int>& contacts = userIt->getContactIDs();
    std::vector<int>::const_iterator cit;
    for (cit = contacts.begin(); cit != contacts.end(); cit++) {
      int contactID = *cit;
      const std::string& contactUsername = users[contactID].getUsername();
      const std::string& contactName = users[contactID].getName();
      Status contactStatus = users[contactID].getStatus();
      // Allocate buffer for sending the message to the client.
      /* DBG: Perhaps this would be more elegant if it was done by a
       * separate socket function, although there's a danger of having
       * too many overloaded functions in the socket. */
      int messageLength;
      MessageType messageType = mSendContact;
      messageLength = sizeof(messageLength) + sizeof(messageType) +
	sizeof(contactID) +
	contactUsername.length() + 1 + contactName.length() + 1 +
	sizeof(contactStatus);
      char* buffer = new char[messageLength]();
      char* buf = buffer;
      std::memcpy(buf, &messageLength, sizeof(messageLength));
      buf += sizeof(messageLength);
      std::memcpy(buf, &messageType, sizeof(messageType));
      buf += sizeof(messageType);
      std::memcpy(buf, &contactID, sizeof(contactID));
      buf += sizeof(contactID);
      std::memcpy(buf, contactUsername.c_str(), contactUsername.length());
      buf += sizeof(contactUsername.length() + 1);
      std::memcpy(buf, contactName.c_str(), contactName.length());
      buf += sizeof(contactName.length() + 1);
      std::memcpy(buf, &contactStatus, sizeof(contactStatus));
      clientSocket->send(buffer);
      delete []buffer;
    }
  }
}

void ServerProcess::logout(Socket* clientSocket, const std::string& username) {
  /*
   * Set the pointer for this user's client socket as 0, set the user
   * status as offline, and inform contacts to update their status flags
   * for the user.
   */
  std::vector<User>::iterator userIt;
  for (userIt = users.begin(); userIt != users.end(); userIt++) {
    if (username == userIt->getUsername()) {
      // The user has been found, so update the corresponding fields in
      // the entry.
      userIt->setClientSocket(0);
      userIt->setStatus(offline);
      // Iterate through the list of contacts and inform other users
      // that this one has logged out.
      const std::vector<int>& rContactIDs = userIt->getContactIDs();
      std::vector<int>::const_iterator contactsIt;
      for (contactsIt = rContactIDs.begin(); contactsIt != rContactIDs.end(); contactsIt++) {
	/* DBG: This is going to be extremely inefficient as it involves
	 * double loop!  There should be a better way to store user
	 * objects so that the process ID is retrieved automatically
	 * with user ID. */
	std::vector<User>::iterator innerUserIt;
	for (innerUserIt = users.begin(); innerUserIt != users.end(); innerUserIt++) {
	  int contactID = innerUserIt->getUserID();
	  if (contactID == *contactsIt) {
	    /* If the contact has been found, send it a message about
	     * the user being logged out. */
	    // Get the client socket file descriptor.
	    Socket* contactSocket = innerUserIt->getClientSocket();
	    contactSocket->send(mLogout, userIt->getUsername());
	    break;
	  }
	}
      }
      break;
    }
  }
}

int ServerProcess::checkUsername(const std::string& username) const {
  /*
   * Iterate through the vector of users and check if a user with the
   * same name already exists.  If the username is not taken, return 0
   * to the calling process indicating that the chosen username is
   * acceptable.  If the name has already been taken, return 1.
   */
  std::vector<User>::const_iterator userIt;
  for (userIt = users.begin(); userIt != users.end(); userIt++) {
    std::string storedUsername = userIt->getUsername();
    if (username == storedUsername)
      break;
  }
  /*
   * Allow the username (return 0) if the iterator reached the end of
   * the vector without finding the match.  If the match was found, the
   * loop is over before reaching the end of the vector.  Return 1 to
   * indicate that the username has already been taken.
   */
  if (userIt == users.end())
    return 0;
  else
    return 1;
}

/*
 * This function will receive a reference to the vector of strings in
 * which user data is packed in the following order:
 * 1. user full name
 * 2. username
 * 3. plain text user password
 */
void ServerProcess::addUser(Socket* clientSocket, const std::vector<std::string>& userDetails) {
  std::string name = userDetails[0];
  std::string username = userDetails[1];
  std::string password = userDetails[2];
  /*
   * The password is still in plain text.  It should be encrypted and
   * prepended with salt before storing.
   *
   * DBG: Note: it seems to be a bad idea to send passwords unencrypted,
   * so a suggested improvement to this approach seems to be to send
   * some key that a client can use to encrypt the password before
   * sending it to the server.  This would require changing this part of
   * the function.
   */
  const char* saltCandidates = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";
  char salt[2];
  // Create salt from the first two randomly chosen characters from the
  // set.
  salt[0] = saltCandidates[saltDistribution(saltGenerator)];
  salt[1] = saltCandidates[saltDistribution(saltGenerator)];
  char* encryptedPasswordContents = crypt(password.c_str(), salt);
  std::string encryptedPassword(encryptedPasswordContents);
  int userID = 0;
  if (users.size() > 0)
    userID = users.back().getUserID() + 1;
  // empty string for contacts as the last argument
  users.push_back(User(userID, username, name, encryptedPassword, clientSocket, online, ""));

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
