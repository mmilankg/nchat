#ifndef PROCESS_H
#define PROCESS_H

#include <sys/types.h>
#include <unistd.h>

// base class for describing a Linux process
class Process {
  protected:
  pid_t pid;	// process identifier
  public:
  Process() { pid = ::getpid(); }
  virtual ~Process() { }
  virtual int run() = 0;
  pid_t getpid() const { return pid; }
};

#endif	// PROCESS_H
