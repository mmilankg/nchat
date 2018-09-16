#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include "dialog.h"
#include "signupDialog.h"

// derived class for describing login panel
class LoginDialog : public Dialog {
  // pointer to the socket to the server
  const Socket* pSocket;
  public:
  LoginDialog(const Socket* pSock, int h = 9, int w = 35, int y = 0, int x = 0);
  virtual ~LoginDialog() { }
};

// class for defining the action associated with the login button
class LoginItem : public NCursesMenuItem {
  LoginDialog* pParentPanel;
  const Socket* pSocket;
  public:
  LoginItem(LoginDialog* pPanel, const Socket* pSock) :
    NCursesMenuItem("Log in    "),
    pParentPanel(pPanel),
    pSocket(pSock)
  { }

  bool action();
};

// class for the signup button action
class SignupItem : public NCursesMenuItem {
  const Socket* pSocket;
  public:
  SignupItem(const Socket* pSock) : NCursesMenuItem("Sign up   "), pSocket(pSock) { }
  bool action() {
    SignupDialog signupDialog(pSocket);
    signupDialog.run();
    return false;
  }
};

// class for the quit button action (copied from ncurses demo.cc)
class QuitItem : public NCursesMenuItem {
  const Socket* pSocket;
  public:
  QuitItem(const Socket* pSock) : NCursesMenuItem("Quit      "), pSocket(pSock) { }
  bool action() {
    // Send the server the quit command.
    pSocket->send(cQuit);
    return true;
  }
};

#endif	// LOGINDIALOG_H
