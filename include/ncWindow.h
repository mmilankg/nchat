#ifndef NCWINDOW_H
#define NCWINDOW_H

#include "socket.h"

/*
 * This class will represent the main window presented to the user
 * when they log in or after the successful sign up process.
 *
 * The design of the class is inspired by the SillyDemo class of the
 * ncurses C++ demo program, which doesn't have any members, but
 * other ncurses elements are started when the object of this class
 * is created and its run() function invoked.
 */
class NCWindow {
  Socket* pSocket;
  public:
  NCWindow(Socket* pSock) : pSocket(pSock) { };
  void run();
};

#endif	// NCWINDOW_H
