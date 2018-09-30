#ifndef TOPMENU_H
#define TOPMENU_H

#include "dialog.h"
#include "socket.h"
#include <cursesm.h>

class FindContactDialog : public Dialog {
  Socket* pSocket;
  public:
  FindContactDialog(Socket* pSock, int h = 6, int w = 35, int y = 0, int x = 0);
  virtual ~FindContactDialog() { }
};

/* DBG: This looks very similar to the SignupOKItem, so perhaps it
 * would be better to replace both with a DialogOKItem. */
class FindContactOKItem : public NCursesMenuItem {
  FindContactDialog* pParentPanel;
  Socket* pSocket;

  public:
  FindContactOKItem(FindContactDialog* pPanel, Socket* pSock) :
    NCursesMenuItem("       OK       "),
    pParentPanel(pPanel),
    pSocket(pSock)
  { }

  bool action();
};

/* DBG: again very similar to SignupCancelItem, so better taken into the
 * Dialog class. */
class FindContactCancelItem : public NCursesMenuItem {
  public:
  FindContactCancelItem() : NCursesMenuItem("     Cancel     ") { }
  bool action() { return true; }
};

class ContactRequestReturnDialog : public Dialog {
  public:
  ContactRequestReturnDialog(
      const std::string& contactUsername,
      int serverResponse,
      int h = 4, int w = 35, int y = 0, int x = 0
      );
  virtual ~ContactRequestReturnDialog() { }
};

class FindContactItem : public NCursesMenuItem {
  Socket* pSocket;
  public:
  FindContactItem(const char* pTitle, Socket* pSock) : NCursesMenuItem(pTitle), pSocket(pSock) { }

  bool action() {
    FindContactDialog findContactDialog(pSocket);
    findContactDialog.run();
    return false;
  }
};

class ContactsMenu : public NCursesMenu {
  NCursesMenuItem** paItems;
  Socket* pSocket;
  enum { nItems = 1 };

  public:
  ContactsMenu(Socket* pSock) : NCursesMenu(nItems, 14, 1, 0), paItems(0), pSocket(pSock) {
    paItems = new NCursesMenuItem*[1 + nItems];
    paItems[0] = new FindContactItem("Find contact ", pSocket);
    paItems[1] = new NCursesMenuItem();	// empty item terminator

    InitMenu(paItems, false, true);
  }
};

class ContactsItem : public NCursesMenuItem {
  Socket* pSocket;

  public:
  ContactsItem(const char* pTitle, Socket* pSock) : NCursesMenuItem(pTitle), pSocket(pSock) { }

  bool action() {
    ContactsMenu contactsMenu(pSocket);
    contactsMenu();
    return false;
  }
};

class LogoutItem : public NCursesMenuItem {
  Socket* pSocket;
  bool logout;

  public:
  LogoutItem(const char* pTitle, Socket* pSock) : NCursesMenuItem(pTitle), pSocket(pSock), logout(false) { }

  bool action() {
    logout = true;
    return true;
  }
  bool getLogoutStatus() const { return logout; }
};

class QuitItem : public NCursesMenuItem {
  Socket* pSocket;
  bool quit;

  public:
  QuitItem(const char* pTitle, Socket* pSock) : NCursesMenuItem(pTitle), pSocket(pSock), quit(false) { }

  bool action() {
    quit = true;
    return true;
  }
  bool getQuitStatus() const { return quit; }
};

class AccountMenu : public NCursesMenu {
  NCursesMenuItem** paItems;
  Socket* pSocket;
  enum { nItems = 2 };

  public:
  AccountMenu(Socket* pSock) :
    NCursesMenu(nItems, 9, 1, 12), paItems(0), pSocket(pSock) {
    paItems = new NCursesMenuItem*[1 + nItems];
    paItems[0] = new LogoutItem("Log out ", pSocket);
    paItems[1] = new QuitItem("Quit", pSocket);
    paItems[2] = new NCursesMenuItem();

    InitMenu(paItems, false, true);
  }
  // DBG: Are these constructs OK for downcasting pointers?
  bool getLogoutStatus() const { return ((LogoutItem*)paItems[0])->getLogoutStatus(); }
  bool getQuitStatus() const { return ((QuitItem*)paItems[1])->getQuitStatus(); }
};

class AccountItem : public NCursesMenuItem {
  Socket* pSocket;
  bool logout;
  bool quit;

  public:
  AccountItem(const char* pTitle, Socket* pSock) :
    NCursesMenuItem(pTitle), pSocket(pSock),
    logout(false), quit(false)
  { }

  bool action() {
    AccountMenu accountMenu(pSocket);
    accountMenu();
    logout = accountMenu.getLogoutStatus();
    quit = accountMenu.getQuitStatus();
    return logout || quit;
  }
  bool getLogoutStatus() const { return logout; }
  bool getQuitStatus() const { return quit; }
};

class TopMenu : public NCursesMenu {
  NCursesMenuItem** paItems;
  Socket* pSocket;
  enum { nItems = 2 };

  public:
  TopMenu(Socket* pSock) : NCursesMenu(1, 10 * nItems + 2, 0, 0), paItems(0), pSocket(pSock) {
    paItems = new NCursesMenuItem*[1 + nItems];
    paItems[0] = new ContactsItem("Contacts  ", pSocket);
    paItems[1] = new AccountItem("Account  ", pSocket);
    paItems[2] = new NCursesMenuItem();	// empty item terminator

    InitMenu(paItems, false, true);
    set_format(1, nItems);
  }

  bool getLogoutStatus() const { return ((AccountItem*)paItems[1])->getLogoutStatus(); }
  bool getQuitStatus() const { return ((AccountItem*)paItems[1])->getQuitStatus(); }
};

#endif	// TOPMENU_H
