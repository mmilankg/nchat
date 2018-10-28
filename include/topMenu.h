#ifndef TOPMENU_H
#define TOPMENU_H

#include "dialog.h"
#include "message.h"
#include "socket.h"
#include <cursesm.h>
#include <string>

/* DBG: Is this declaration allowed or maybe considered bad practice?  A better way seem to #include "ncWindow.h", but
 * that file already includes this one. */
class NCWindow;
class TopMenu;

class FindContactDialog : public Dialog {
    Socket * pSocket;

public:
    FindContactDialog(Socket * pSock, int h = 6, int w = 35, int y = 0, int x = 0);
    virtual ~FindContactDialog() {}
};

/* DBG: This looks very similar to the SignupOKItem, so perhaps it would be better to replace both with a DialogOKItem.
 */
class FindContactOKItem : public NCursesMenuItem {
    FindContactDialog * pParentPanel;
    Socket *            pSocket;

public:
    FindContactOKItem(FindContactDialog * pPanel, Socket * pSock) :
        NCursesMenuItem("       OK       "),
        pParentPanel(pPanel),
        pSocket(pSock)
    {
    }

    bool action();
};

/* DBG: again very similar to SignupCancelItem, so better taken into the Dialog class. */
class FindContactCancelItem : public NCursesMenuItem {
public:
    FindContactCancelItem() : NCursesMenuItem("     Cancel     ") {}
    bool action() { return true; }
};

class FindContactItem : public NCursesMenuItem {
    Socket * pSocket;

public:
    FindContactItem(const char * pTitle, Socket * pSock) : NCursesMenuItem(pTitle), pSocket(pSock) {}

    bool action()
    {
        FindContactDialog findContactDialog(pSocket);
        findContactDialog.run();
        return true;
    }
};

class ContactsMenu : public NCursesMenu {
    NCursesMenuItem ** paItems;
    Socket *           pSocket;
    enum { nItems = 1 };

public:
    ContactsMenu(Socket * pSock) : NCursesMenu(nItems, 14, 1, 0), paItems(0), pSocket(pSock)
    {
        paItems    = new NCursesMenuItem *[1 + nItems];
        paItems[0] = new FindContactItem("Find contact ", pSocket);
        paItems[1] = new NCursesMenuItem(); // empty item terminator

        InitMenu(paItems, false, true);
    }
};

class ContactsItem : public NCursesMenuItem {
    Socket * pSocket;

public:
    ContactsItem(const char * pTitle, Socket * pSock) : NCursesMenuItem(pTitle), pSocket(pSock) {}

    bool action()
    {
        ContactsMenu contactsMenu(pSocket);
        contactsMenu();
        return false;
    }
};

class LogoutItem : public NCursesMenuItem {
    // pointer to the parent top menu
    TopMenu * pTopMenu;
    Socket *  pSocket;
    bool      logout;

public:
    LogoutItem(const char * pTitle, TopMenu * pTop, Socket * pSock) :
        NCursesMenuItem(pTitle),
        pTopMenu(pTop),
        pSocket(pSock),
        logout(false)
    {
    }

    bool action();
    bool getLogoutStatus() const { return logout; }
};

class ExitItem : public NCursesMenuItem {
    Socket * pSocket;
    bool     exit;

public:
    ExitItem(const char * pTitle, Socket * pSock) : NCursesMenuItem(pTitle), pSocket(pSock), exit(false) {}

    bool action()
    {
        exit = true;
        return true;
    }
    bool getExitStatus() const { return exit; }
};

class AccountMenu : public NCursesMenu {
    // pointer to the parent top menu
    TopMenu *          pTopMenu;
    NCursesMenuItem ** paItems;
    Socket *           pSocket;
    enum { nItems = 2 };

public:
    AccountMenu(TopMenu * pTop, Socket * pSock) :
        NCursesMenu(nItems, 9, 1, 12),
        pTopMenu(pTop),
        paItems(0),
        pSocket(pSock)
    {
        paItems    = new NCursesMenuItem *[1 + nItems];
        paItems[0] = new LogoutItem("Log out ", pTopMenu, pSocket);
        paItems[1] = new ExitItem("Exit", pSocket);
        paItems[2] = new NCursesMenuItem();

        InitMenu(paItems, false, true);
    }
    // DBG: Are these constructs OK for downcasting pointers?
    bool getLogoutStatus() const { return ((LogoutItem *)paItems[0])->getLogoutStatus(); }
    bool getExitStatus() const { return ((ExitItem *)paItems[1])->getExitStatus(); }
};

class AccountItem : public NCursesMenuItem {
    // pointer to the parent top menu
    TopMenu * pTopMenu;
    Socket *  pSocket;
    bool      logout;
    bool      exit;

public:
    AccountItem(const char * pTitle, TopMenu * pTop, Socket * pSock) :
        NCursesMenuItem(pTitle),
        pTopMenu(pTop),
        pSocket(pSock),
        logout(false),
        exit(false)
    {
    }

    bool action()
    {
        AccountMenu accountMenu(pTopMenu, pSocket);
        accountMenu();
        logout = accountMenu.getLogoutStatus();
        exit   = accountMenu.getExitStatus();
        return logout || exit;
    }
    bool getLogoutStatus() const { return logout; }
    bool getExitStatus() const { return exit; }
};

class TopMenu : public NCursesMenu {
    // pointer to the parent window
    NCWindow *         pNCWindow;
    NCursesMenuItem ** paItems;
    Socket *           pSocket;
    enum { nItems = 2 };

public:
    TopMenu(NCWindow * pNCWin, Socket * pSock) :
        pNCWindow(pNCWin),
        NCursesMenu(1, 10 * nItems + 2, 0, 0),
        paItems(0),
        pSocket(pSock)
    {
        paItems    = new NCursesMenuItem *[1 + nItems];
        paItems[0] = new ContactsItem("Contacts  ", pSocket);
        paItems[1] = new AccountItem("Account  ", this, pSocket);
        paItems[2] = new NCursesMenuItem(); // empty item terminator

        InitMenu(paItems, false, true);
        set_format(1, nItems);
    }

    bool getLogoutStatus() const { return ((AccountItem *)paItems[1])->getLogoutStatus(); }
    bool getExitStatus() const { return ((AccountItem *)paItems[1])->getExitStatus(); }
    void handleKey(int key);

    NCWindow * getNCWindow() const { return pNCWindow; }
};

#endif // TOPMENU_H
