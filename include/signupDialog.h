#ifndef SIGNUPDIALOG_H
#define SIGNUPDIALOG_H

#include "dialog.h"
#include "socket.h"
#include "commands.h"

// derived class for describing login panel
class SignupDialog : public Dialog {
  public:
  /*
   * Constructor: uses dialog dimensions and positions along with
   * the socket address that is passed into the Signup OK button.
   */
  SignupDialog(const Socket* pSock, int h = 12, int w = 35, int y = 0, int x = 0);
  virtual ~SignupDialog() { }
};

// class for defining the action associated with the login button
class SignupOKItem : public NCursesMenuItem {
  SignupDialog* pParentPanel;
  const Socket* pSocket;
  public:
  SignupOKItem(SignupDialog* pPanel, const Socket* pSock) :
    NCursesMenuItem("OK              "),
    pParentPanel(pPanel),
    pSocket(pSock)
  { }

  // function activated when the OK button is pressed
  bool action();
};

// class for the signup button action
class SignupCancelItem : public NCursesMenuItem {
  public:
  SignupCancelItem() : NCursesMenuItem("Cancel          ") { }
  bool action() { return true; }
};

#endif	// SIGNUPDIALOG_H
