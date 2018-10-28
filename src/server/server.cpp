#include "acceptor.h"
#include "message.h"
#include "server.h"
#include "trace.h"
#include <signal.h>
#include <cassert>
#include <chrono>

extern int verbosityLevel;

Server::Server() :
    usersFileName("nchatUsers"),
    // Open the file for reading and writing in the append mode.
    usersFile(usersFileName.c_str(), std::ios::in | std::ios::out | std::ios::app),
    // Create the Acceptor object for listening to incoming connection requests.
    acceptor(this),
    // Initialize the salt generator using the current time.
    saltGenerator(std::chrono::system_clock::now().time_since_epoch().count()),
    // Initialize the random distribution object.
    saltDistribution(std::uniform_int_distribution<>(0, 63))
{
    TRACE(verbosityLevel, "server process constructor")
    // Open the file for reading and writing in the append mode.
    // usersFile.open(usersFileName.c_str(), std::ios::in | std::ios::out | std::ios::app);
    // Parse the user file line by line.
    std::string line;
    while (getline(usersFile, line)) {
        std::istringstream ss(line);
        std::string        field;
        /*
         * Parse the selected line field by field.  Fields are separated by a colon.
         * 1. user id (int)
         * 2. username (string)
         * 3. real name (string)
         * 4. encrypted user password (string; first two characters are salt)
         * 5. list of contacts (integers separated by commas)
         * 6. list of sent contact requests (integers separated by commas)
         * 7. list of received contact requests (integers separated by commas)
         */
        int         userID;
        std::string username;
        std::string name;
        std::string password;
        std::string contacts;
        std::string sentContactRequests;
        std::string receivedContactRequests;
        getline(ss, field, ':');
        userID = atoi(field.c_str());
        getline(ss, username, ':');
        getline(ss, name, ':');
        getline(ss, password, ':');
        getline(ss, contacts, ':');
        getline(ss, sentContactRequests, ':');
        getline(ss, receivedContactRequests, ':');
        users.push_back(
            User(userID, username, name, password, 0, offline, contacts, sentContactRequests, receivedContactRequests));
    }
    /*
     * Clear the error state flag after reading in order to be able to write to the file in other functions:
     *
     * https://stackoverflow.com/questions/32435991/how-to-read-and-write-in-file-with-fstream-simultaneously-in-c/32437476#32437476
     */
    usersFile.clear();
    TRACE(verbosityLevel, "server process constructor completed")
}

int Server::run()
{
    /* Ignore the SIGPIPE signal, but later add the functionality to clean up the client if the connection is broken. */
    struct sigaction signalAction;
    signalAction.sa_handler = SIG_IGN;
    // sigaction(SIGPIPE, &signalAction, 0);

    while (true) {
        /* Prepare for the select() call so that the listening at a socket doesn't completely block the execution. */
        fd_set socketDescriptors;
        FD_ZERO(&socketDescriptors);
        int maxSfd = acceptor.getSfd();
        FD_SET(acceptor.getSfd(), &socketDescriptors);
        for (auto connection : connections) {
            int connectionSfd = connection->getSfd();
            FD_SET(connectionSfd, &socketDescriptors);
            if (connectionSfd > maxSfd) maxSfd = connectionSfd;
        }

        /*
         * Start the select() function, but only for reading when sockets are ready.  Set the last value to 0 in order
         * to listen indefinitely.
         */
        select(maxSfd + 1, &socketDescriptors, 0, 0, 0);

        // when listening socket is ready to accept
        if (FD_ISSET(acceptor.getSfd(), &socketDescriptors)) {
            acceptor.acceptConnection();
            TRACE(verbosityLevel, "connection accepted")
        }
        // for messages on client sockets
        else {
            // Loop through all connections to see if their client sockets are selected.
            for (auto connection : connections) {
                int clientSocketFD = connection->getSfd();
                assert(clientSocketFD > 0);
                if (FD_ISSET(clientSocketFD, &socketDescriptors)) connection->receive();
            }
        }
    }

    return 0;
}

void Server::createConnection(Socket * pSocket)
{
    Connection * pConnection = new Connection(this, pSocket);
    connections.push_back(pConnection);
}

void Server::react(Connection * connection, MessageType messageType, const std::vector<char> & message)
{
    // Process the message.
    switch (messageType) {
    case mSignup: {
        std::vector<std::string> userDetails;
        bufferToStrings(message, userDetails);
        signup(connection, userDetails);
        break;
    }
    case mLogin: {
        std::vector<std::string> userDetails;
        bufferToStrings(message, userDetails);
        login(connection, userDetails);
        break;
    }
    case mLogout: {
        std::string username = message.data();
        logout(connection, username);
        break;
    }
    case mFindUser: {
        std::string requestedUsername = message.data();
        findUser(connection, requestedUsername);
        break;
    }
    case mQuit: {
        quit(connection);
        break;
    }
    }
}

void Server::signup(Connection * connection, const std::vector<std::string> & userDetails)
{
    // Unpack the name, username and password from the message.
    std::string name = userDetails[0];
    assert(name.length() > 0);
    std::string username = userDetails[1];
    assert(username.length() > 0);
    std::string password = userDetails[2];
    assert(password.length() > 0);

    int response; // response to the client
    if (checkUsername(username) == 0) {
        response = 0;
        addUser(connection, userDetails);
    }
    else
        response = 1;

    // Return the response to the client.
    MessageType messageType = mLogin;
    connection->transmit(messageType, response);
}

void Server::login(Connection * connection, const std::vector<std::string> & userDetails)
{
    // Unpack the username and password from the message.
    std::string username = userDetails[0];
    assert(username.length() > 0);
    std::string password = userDetails[1];
    assert(password.length() > 0);
    /*
     * While the range-based loop seems like a great feature, I decided to leave the iterator approach in this
     * particular example because this loop is designed to break when the match is found and the last iterator position
     * is used outside of the loop and has to stay defined when the loop is finished, which probably doesn't happen with
     * range-based loops.
     */
    std::vector<User>::iterator userIterator;
    int                         response;
    for (userIterator = users.begin(); userIterator != users.end(); userIterator++) {
        std::string storedUsername = userIterator->getUsername();
        // When the user is found:
        if (username == storedUsername) {
            // Check the password.  Take the stored password from the database and extract the first two characters as
            // salt.
            std::string storedPassword = userIterator->getPassword();
            char        salt[2];
            std::memcpy(salt, storedPassword.c_str(), 2);
            char *      encryptedPasswordContents = crypt(password.c_str(), salt);
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
     * If the loop finishes without finding the user (iterator == vector.end()), the username that was sent is wrong as
     * no such user has been found in the system.  Return response 2 to the client.
     */
    if (userIterator == users.end()) response = 2;

    MessageType messageType = mLogin;
    connection->transmit(messageType, response);

    /* If the user is logged in, set their status to online and send them the list of contacts. */
    if (response == 0) {
        // Mark the user's status in the list of all users as online.
        userIterator->setStatus(online);
        // Set the connection for this user.
        userIterator->setConnection(connection);

        // Send the list of contacts to the user.
        const std::vector<int> & contacts = userIterator->getContactIDs();
        for (auto contactID : contacts) {
            const std::string & contactUsername = users[contactID].getUsername();
            const std::string & contactName     = users[contactID].getName();
            Status              contactStatus   = users[contactID].getStatus();
            // Allocate buffer for sending the message to the client.
            MessageType messageType  = mSendContact;
            int messageContentLength = sizeof(contactID) + contactUsername.length() + 1 + contactName.length() + 1
                                       + sizeof(contactStatus);
            std::vector<char> messageContent;
            messageContent.resize(messageContentLength);
            int nBytesProcessed = 0;
            std::memcpy(messageContent.data() + nBytesProcessed, &contactID, sizeof(contactID));
            nBytesProcessed += sizeof(contactID);
            std::memcpy(messageContent.data() + nBytesProcessed, contactUsername.c_str(), contactUsername.length());
            nBytesProcessed += sizeof(contactUsername.length() + 1);
            std::memcpy(messageContent.data() + nBytesProcessed, contactName.c_str(), contactName.length());
            nBytesProcessed += sizeof(contactName.length() + 1);
            std::memcpy(messageContent.data() + nBytesProcessed, &contactStatus, sizeof(contactStatus));
            connection->transmit(messageType, messageContent);
        }
    }
}

void Server::logout(Connection * connection, const std::string & username)
{
    assert(username.length() > 0);
    /*
     * Set the pointer for this user's connection object as 0, set the user status as offline, and inform contacts to
     * update their status flags for the user.
     */
    for (auto user : users) {
        if (username == user.getUsername()) {
            // The user has been found, so update the corresponding fields in the entry.
            user.setConnection(0);
            user.setStatus(offline);
            // Iterate through the list of contacts and inform other users that this one has logged out.
            const std::vector<int> & rContactIDs = user.getContactIDs();
            for (auto contactID : rContactIDs) {
                /* DBG: This is going to be extremely inefficient as it involves double loop!  There should be a better
                 * way to store user objects so that the connection pointer is retrieved automatically with user ID. */
                for (auto innerUser : users) {
                    int innerUserContactID = innerUser.getUserID();
                    if (innerUserContactID == contactID) {
                        /* If the contact has been found, send it a message about the user being logged out. */
                        Connection * innerUserConnection = innerUser.getConnection();
                        innerUserConnection->transmit(mLogout, user.getUsername());
                        break;
                    }
                }
            }
            break;
        }
    }
}

void Server::quit(Connection * connection)
{
    // Remove the connection from the vector of connections.
    auto iterator = connections.begin();
    while (iterator != connections.end()) {
        if (*iterator == connection) {
            /* DBG: check if the erase() function also invokes the connection destructor. */
            connections.erase(iterator);
            break;
        }
        iterator++;
    }
}

int Server::checkUsername(const std::string & username) const
{
    assert(username.length() > 0);
    /*
     * Iterate through the vector of users and check if a user with the same name already exists.  If the username is
     * not taken, return 0 to the calling process indicating that the chosen username is acceptable.  If the name has
     * already been taken, return 1.
     */
    for (auto user : users) {
        std::string storedUsername = user.getUsername();
        // Return 1 if the match was found.
        if (username == storedUsername) return 1;
    }
    /*
     * Allow the username (return 0) if the iterator reached the end of the vector without finding the match.  If the
     * match was found, the the return 1 was already executed and the function never reaches this point.
     */
    return 0;
}

/*
 * This function will receive a reference to the vector of strings in which user data is packed in the following order:
 * 1. user full name
 * 2. username
 * 3. plain text user password
 */
void Server::addUser(Connection * connection, const std::vector<std::string> & userDetails)
{
    std::string name = userDetails[0];
    assert(name.length() > 0);
    std::string username = userDetails[1];
    assert(username.length() > 0);
    std::string password = userDetails[2];
    assert(password.length() > 0);
    /*
     * The password is still in plain text.  It should be encrypted and prepended with salt before storing.
     *
     * DBG: Note: it seems to be a bad idea to send passwords unencrypted, so a suggested improvement to this approach
     * seems to be to send some key that a client can use to encrypt the password before sending it to the server.  This
     * would require changing this part of the function.
     */
    const char * saltCandidates = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";
    char         salt[2];
    // Create salt from the first two randomly chosen characters from the set.
    salt[0]                               = saltCandidates[saltDistribution(saltGenerator)];
    salt[1]                               = saltCandidates[saltDistribution(saltGenerator)];
    char *      encryptedPasswordContents = crypt(password.c_str(), salt);
    std::string encryptedPassword(encryptedPasswordContents);
    int         userID = 0;
    if (users.size() > 0) userID = users.back().getUserID() + 1;
    // empty string for contacts as the last argument
    users.push_back(User(userID, username, name, encryptedPassword, connection, online, "", "", ""));

    /*
     * Add the user to the file listing all users and their contacts.  This should be done with the locking mechanism so
     * that the process has an exclusive access to the file while the write operation is ongoing.  This is left for
     * later implementation.
     */
    /*
     * Pack user information into a single line with the following format: userID:name:username:encryptedPassword:
     * These are separated by colons.  The last colon is the end of the line for a new user.  For an existing user, it
     * will be followed by the list of contacts: comma-separated userIDs.
     */
    std::string line = std::to_string(userID) + ":";
    line += username + ":";
    line += name + ":";
    line += encryptedPassword + ":";
    usersFile << line << std::endl;
}

void Server::findUser(Connection * connection, const std::string & requestedUsername)
{
    assert(requestedUsername.length() > 0);
    /*
     * This function is invoked when a user tries to find another user in order to establish a contact.  The server will
     * try to find the requested user in its list of all users.
     *
     * If the requested user is found, the server sends the sending user a message that the contact has been found, and
     * the requested user a message that asks for the contact to be established.  The User object for both users will
     * also be updated to include the ID of the other user in the sentContactRequests field for the sending user and the
     * receivedContactRequests field for the requested user.
     *
     * If the requested user is not found, the server sends the sending user a response 1 indicating that the contact
     * hasn't been found.
     *
     * The server doesn't allow a user to request a contact with themselves and returns a response 2 to indicate that to
     * the client.
     */
    int                         serverResponse;
    std::vector<User>::iterator requestedUserIt;
    for (requestedUserIt = users.begin(); requestedUserIt != users.end(); requestedUserIt++) {
        if (requestedUsername == requestedUserIt->getUsername()) {
            // Send the requesting user response 0.
            serverResponse = 0;
            // The requested user has been found.  Find the sending user from its connection pointer.
            std::vector<User>::iterator sendingUserIt;
            for (sendingUserIt = users.begin(); sendingUserIt != users.end(); sendingUserIt++)
                if (connection == sendingUserIt->getConnection()) break;
            // It's not allowed for a user to establish a self-contact.
            if (requestedUserIt == sendingUserIt) {
                serverResponse = 2;
                break;
            }
            int                sendingUserID         = sendingUserIt->getUserID();
            int                requestedUserID       = requestedUserIt->getUserID();
            std::vector<int> & sentContactRequestIDs = requestedUserIt->getSentContactRequestIDs();
            sentContactRequestIDs.push_back(requestedUserID);
            std::vector<int> & receivedContactRequestIDs = requestedUserIt->getReceivedContactRequestIDs();
            receivedContactRequestIDs.push_back(sendingUserID);
            Connection * requestedUserConnection = requestedUserIt->getConnection();
            if (requestedUserConnection != 0) {
                std::string sendingUsername = sendingUserIt->getUsername();
                requestedUserConnection->transmit(mContactRequest, sendingUsername);
            }
            break;
        }
    }

    // The requested user hasn't been found.  Send the requesting user response 1.
    if (requestedUserIt == users.end()) serverResponse = 1;

    int               messageContentLength = sizeof(serverResponse) + requestedUsername.length() + 1;
    std::vector<char> messageContent;
    messageContent.resize(messageContentLength);
    std::memcpy(messageContent.data(), &serverResponse, sizeof(serverResponse));
    std::memcpy(messageContent.data() + sizeof(serverResponse), requestedUsername.c_str(), requestedUsername.length());
    connection->transmit(mFindUser, messageContent);
}

void Server::bufferToStrings(char * buffer, int bufferLength, std::vector<std::string> & strings) const
{
    int    remainingLength = bufferLength;
    char * buf             = buffer;
    // Process each string terminated with null-byte until the
    // remainingLength drops to 0.
    while (remainingLength > 0) {
        strings.push_back(buf);
        buf += strings.back().length();
        remainingLength -= strings.back().length();
        // Process the null-byte.
        buf++;
        remainingLength--;
    }
    // Check the remaining length hasn't become negative.
    assert(remainingLength == 0);
}

void Server::bufferToStrings(const std::vector<char> & buffer, std::vector<std::string> & strings) const
{
    int nBytesProcessed = 0;
    while (nBytesProcessed < buffer.size()) {
        strings.push_back(buffer.data() + nBytesProcessed);
        nBytesProcessed += strings.back().length() + 1;
    }
}
