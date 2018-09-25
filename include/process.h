#ifndef PROCESS_H
#define PROCESS_H

#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <vector>

// base class for describing a Linux process
class Process {
  protected:
  pid_t pid;	// process identifier
  public:
  Process() { pid = ::getpid(); }
  virtual ~Process() { }
  virtual int run() = 0;
  pid_t getpid() const { return pid; }
  // Perhaps not the best place for this function, but it's going to be
  // used extensively in objects of the ServerProcess class.
  void bufferToStrings(char* buffer, int bufferLength, std::vector<std::string>& strings) const;
};

#endif	// PROCESS_H
