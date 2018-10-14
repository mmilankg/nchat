#ifndef TRACE_H
#define TRACE_H

#include <iostream>

/* TRACE macro for following program execution. */
#define TRACE(level, message) \
  if (level == 1) \
    std::cout << "Passing through " << __FILE__ << ": " << message << "." << std::endl; \
  else if (level == 2) \
    std::cout << "Passing through " << __FILE__ << " (line " << __LINE__ << "): " << message << "." << std::endl;

#endif // TRACE_H
