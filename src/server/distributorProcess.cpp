#include "distributorProcess.h"

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

    const Message& message = pMessageQueue->front();

    /* Decode the message. */
    MessageType messageType = message.getType();
    pid_t messageSender = message.getSender();
    pid_t messageRecipient = message.getRecipient();
    /* DBG: print message contents! */
    std::cout << "type: " << messageType << std::endl;
    std::cout << "from: " << messageSender << std::endl;
    std::cout << "to: " << messageRecipient << std::endl;
    //std::cout << "contents: " << message.read() << std::endl;
    //std::cout << "contents: " << static_cast<char*>(message.read().get()) << std::endl;
    /* DBG! */
    //char* mRead = new char[message.getLength()];
    //message.read(mRead, message.getLength());
    //message.read(mRead);
    //std::cout << "contents: " << mRead << std::endl;
    // Remove the message from the queue if the recepient matches the process ID.
    char* msg = new char[message.getLength() + 1];
    message.read(msg);
    std::cout << "contents: " << msg << std::endl;
    if (messageRecipient == pid)
      pMessageQueue->pop_front();
  }
}
