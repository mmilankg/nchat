#ifndef MESSAGE_H
#define MESSAGE_H

#include <sys/types.h>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/offset_ptr.hpp>
#include <cstring>

enum MessageType {
  mCheckUsername,   // check if a username exists when signing up a new user
  mAddUser,	    // add the new user to the vector of existing users
  mUsernameStatus,  // return status on the username query for new users
  mSignupName,	    // send the name of the user to be signed up
  mPassword,	    // send the password of the user to be signed up
  mText,	    // text message to another user
  mVoice	    // voice message to another user
};

/*
 * message that is exchanged between server processes
 *
 * As the message has to go into the message queue which resides in the
 * shared memory, its contents are stored as a simple data type (byte
 * array at char*) rather than string.
 */
class Message {
  // process identifiers for the source and destination of the message
  pid_t from, to;
  // message type
  MessageType mType;
  // default length of the message content
  enum { defaultLength = 1024 };
  // message length
  int length;
  // address of the byte array in which the message is stored
  char* contents;
  /* DBG! */
  boost::interprocess::offset_ptr<char> contentsOffsetPtr;
  public:
  Message() : from(0), to(0), length(defaultLength) {
    /*
     * Open the shared memory segment for memory allocation for characters.
     * The segment will be created by the distributor process.
     */
    boost::interprocess::managed_shared_memory messageSegment(boost::interprocess::open_only, "messageQueueMemory");
    // Allocate the memory in the segment.
    contents = messageSegment.construct<char>(boost::interprocess::anonymous_instance)[length](0);
    contentsOffsetPtr = contents;
  }
  Message(pid_t sender, pid_t recipient, MessageType type, int len, const char* msg) :
    from(sender), to(recipient), mType(type), length(len)
  {
    boost::interprocess::managed_shared_memory messageSegment(boost::interprocess::open_only, "messageQueueMemory");
    // Add 1 to the message length in order to complete with a 0-byte if the message is text.
    contents = messageSegment.construct<char>(boost::interprocess::anonymous_instance)[length + 1](0);
    contentsOffsetPtr = contents;
    std::memcpy(contents, msg, length);
    // No need to end with 0-byte as the whole message is prefilled with 0.
  }
  Message(const Message& message) :
    from(message.from), to(message.to), mType(message.mType), length(message.length),
    contents(message.contents),
    contentsOffsetPtr(message.contentsOffsetPtr)  /* DBG! */
  { }
  ~Message() {
    // Release the memory allocated for the character array.
    boost::interprocess::managed_shared_memory messageSegment(boost::interprocess::open_only, "messageQueueMemory");
    //messageSegment.destroy_ptr(contents);
  }
  Message& operator=(const Message& message) {
    from = message.from;
    to = message.to;
    mType = message.mType;
    length = message.length;
    contents = message.contents;
    contentsOffsetPtr = message.contentsOffsetPtr;
    return *this;
  }
  void setSender(pid_t pid) { from = pid; }
  pid_t getSender() const { return from; }
  void setRecipient(pid_t pid) { to = pid; }
  pid_t getRecipient() const { return to; }
  void setType(MessageType mt) { mType = mt; }
  MessageType getType() const { return mType; }
  void setLength(int messageLength) { length = messageLength; }
  int getLength() const { return length; }
  //char* read() const { return contents; }
  /* DBG! */
  char* read() const { return contentsOffsetPtr.get(); }
  /* DBG! */
  //void read(char*& rDst) const {
    //rDst = contentsOffsetPtr;
  //}
  /* DBG! */
  //boost::interprocess::offset_ptr<char> read() const { return contents; }
  void read(char* dst) const {
    boost::interprocess::managed_shared_memory messageSegment(boost::interprocess::open_only, "messageQueueMemory");
    std::memcpy(dst, contents, length);
    std::memset(dst + length, 0, 1);
  }
  void write(const char* src, int messageLength) {
    /* DBG: Does this need to be opened inside the function? */
    boost::interprocess::managed_shared_memory messageSegment(boost::interprocess::open_only, "messageQueueMemory");
    if (length <= messageLength) {
      messageSegment.destroy_ptr(contents);
      /* DBG! */
      //messageSegment.deallocate(contents.get());
      length = messageLength + 1;
      contents = messageSegment.construct<char>(boost::interprocess::anonymous_instance)[length](0);
      contentsOffsetPtr = contents;
    }
    std::memcpy(contents, src, messageLength);
    // Put 0 terminator at the end of the text.
    std::memset(contents + messageLength, 0, 1);
    /* DBG! */
    //std::memcpy(static_cast<char*>(contents.get()), src, messageLength);
    // Put 0 terminator at the end of the text.
    //std::memset(static_cast<char*>(contents.get()) + messageLength, 0, 1);
  }
  void releaseContents() {
    // Release the memory allocated for the character array.
    boost::interprocess::managed_shared_memory messageSegment(boost::interprocess::open_only, "messageQueueMemory");
    messageSegment.destroy_ptr(contents);
  }
};

#endif	// MESSAGE_H
