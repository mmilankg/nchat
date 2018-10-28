#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include "dialog.h"
#include "message.h"
#include "signupDialog.h"

// derived class for describing login panel
class LoginDialog : public Dialog {
    // pointer to the socket to the server
    Socket * pSocket;

public:
    LoginDialog(Socket * pSock, int h = 9, int w = 35, int y = 0, int x = 0);
    virtual ~LoginDialog() {}
};

// class for defining the action associated with the login button
class LoginItem : public NCursesMenuItem {
    LoginDialog * pParentPanel;
    Socket *      pSocket;

public:
    LoginItem(LoginDialog * pPanel, Socket * pSock) :
        NCursesMenuItem("Log in    "),
        pParentPanel(pPanel),
        pSocket(pSock)
    {
    }

    bool action();
};

// class for the signup button action
class SignupItem : public NCursesMenuItem {
    Socket * pSocket;

public:
    SignupItem(Socket * pSock) : NCursesMenuItem("Sign up   "), pSocket(pSock) {}
    bool action()
    {
        SignupDialog signupDialog(pSocket);
        signupDialog.run();
        return false;
    }
};

// class for the quit button action (copied from ncurses demo.cc)
class QuitItem : public NCursesMenuItem {
    Socket * pSocket;

public:
    QuitItem(Socket * pSock) : NCursesMenuItem("Quit      "), pSocket(pSock) {}
    bool action()
    {
        // Send the server the quit command.
        int messageLength = sizeof(messageLength) + sizeof(mQuit);
        pSocket->send(messageLength);
        pSocket->send(mQuit);
        return true;
    }
};

#endif // LOGINDIALOG_H
