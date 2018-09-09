#ifndef DISTRIBUTORPROCESS_H
#define DISTRIBUTORPROCESS_H

#include "message.h"
#include "process.h"
#include "user.h"
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/deque.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>

// STL-compatible allocators for placing vectors and queues in their memory segments
typedef boost::interprocess::allocator<User, boost::interprocess::managed_shared_memory::segment_manager> UserAllocator;
typedef boost::interprocess::allocator<Message, boost::interprocess::managed_shared_memory::segment_manager> MessageAllocator;
// vector of users and message queue types
typedef boost::interprocess::vector<User, UserAllocator> UserVector;
typedef boost::interprocess::deque<Message, MessageAllocator> MessageQueue;

class DistributorProcess : public Process {
  // name of the file with user data (name, password, contacts... (conversation history?))
  std::string usersFileName;

  // memory segments in which the vector of users and queue of messages will be stored
  boost::interprocess::managed_shared_memory* pUserSegment;
  boost::interprocess::managed_shared_memory* pMessageSegment;
  // pointers to STL-compatible allocators
  UserAllocator* pUserAllocator;
  MessageAllocator* pMessageAllocator;
  // pointer to the vector of users and message queue
  UserVector* pUserVector;
  MessageQueue* pMessageQueue;

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
};

#endif	// DISTRIBUTORPROCESS_H
