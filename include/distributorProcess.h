#ifndef DISTRIBUTORPROCESS_H
#define DISTRIBUTORPROCESS_H

#include "message.h"
#include "process.h"
#include "user.h"
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/deque.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <random>

// STL-compatible allocators for placing vectors and queues in their memory segments
typedef boost::interprocess::allocator<User, boost::interprocess::managed_shared_memory::segment_manager> UserAllocator;
typedef boost::interprocess::allocator<Message, boost::interprocess::managed_shared_memory::segment_manager> MessageAllocator;
// vector of users and message queue types
typedef boost::interprocess::vector<User, UserAllocator> UserVector;
typedef boost::interprocess::deque<Message, MessageAllocator> MessageQueue;

class DistributorProcess : public Process {
  // name of the file with user data (name, password, contacts... (conversation history?))
  std::string usersFileName;
  std::fstream usersFile;

  // memory segments in which the vector of users and queue of messages will be stored
  boost::interprocess::managed_shared_memory* pUserSegment;
  boost::interprocess::managed_shared_memory* pMessageSegment;
  // pointers to STL-compatible allocators
  UserAllocator* pUserAllocator;
  MessageAllocator* pMessageAllocator;
  // pointer to the vector of users and message queue
  UserVector* pUserVector;
  MessageQueue* pMessageQueue;
  // random number generator for randomly choosing salt from the list of 64 possible characters
  std::default_random_engine saltGenerator;
  std::uniform_int_distribution<int> saltDistribution;

  public:
  DistributorProcess();
  ~DistributorProcess() {
    // Destroy user vector and message queue.
    pUserSegment->destroy<UserVector>("userVector");
    pMessageSegment->destroy<MessageQueue>("messageQueue");
    // Remove shared memory.
    boost::interprocess::shared_memory_object::remove("userVectorMemory");
    boost::interprocess::shared_memory_object::remove("messageQueueMemory");
    // Release memory assigned to the objects.
    delete pUserAllocator;
    delete pMessageAllocator;
    delete pUserVector;
    delete pMessageQueue;
    delete pUserSegment;
    delete pMessageSegment;
  }
  int run();

  private:
  void checkUsername(int clientProcessID, const std::string& username) const;
  void addUser(const char* userData);
};

#endif	// DISTRIBUTORPROCESS_H
