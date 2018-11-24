#ifndef CONTACTMENU_H
#define CONTACTMENU_H

#include "socket.h"
#include <cursesm.h>
#include <string>

enum ContactType {
    cEstablishedContact, // established contact
    cSentContact,        // sent contact request
    cReceivedContact     // received contact request
};

class ContactMenu;

class TextItem : public NCursesMenuItem {
    ContactMenu * parent;

public:
    TextItem(ContactMenu * pParent, const char * pTitle) : NCursesMenuItem(pTitle), parent(pParent) {}

    bool action();
};

class CallItem : public NCursesMenuItem {
    ContactMenu * parent;

public:
    CallItem(ContactMenu * pParent, const char * pTitle) : NCursesMenuItem(pTitle), parent(pParent) {}

    bool action();
};

class RemoveItem : public NCursesMenuItem {
public:
    RemoveItem(const char * pTitle) : NCursesMenuItem(pTitle) {}
};

class ContactMenu : public NCursesMenu {
private:
    Socket *           pSocket;
    NCursesMenuItem ** paItems;
    enum { nItems = 4 };
    int activatedItem = 0;

public:
    ContactMenu(Socket * pSock, const std::string & username, int position) :
        pSocket(pSock),
        NCursesMenu(1, username.length() + 2 + 7 * nItems + 2, 2 + position, 2),
        paItems(0)
    {
        paItems    = new NCursesMenuItem *[1 + nItems];
        paItems[0] = new NCursesMenuItem(username.c_str());
        paItems[1] = new TextItem(this, "Text   ");
        paItems[2] = new CallItem(this, "Call   ");
        paItems[3] = new RemoveItem("Remove ");
        paItems[4] = new NCursesMenuItem(); // empty item terminator

        paItems[0]->options_off(O_SELECTABLE);
        InitMenu(paItems, false, true);
        set_format(1, nItems);
    }

    Socket * getSocket() const { return pSocket; }
    int      getActivated() const { return activatedItem; }
    void     setActivated(int activated) { activatedItem = activated; }
    void     handleKey(int key);
};

class SentContactRequest : public NCursesMenu {
private:
    NCursesMenuItem ** paItems;
    enum { nItems = 2 };

public:
    SentContactRequest(const std::string & username, int position) :
        NCursesMenu(1, username.length() + 2 + 7 * nItems + 2, 2 + position, 2),
        paItems(0)
    {
        paItems    = new NCursesMenuItem *[1 + nItems];
        paItems[0] = new NCursesMenuItem(username.c_str());
        paItems[1] = new RemoveItem("Revoke ");
        paItems[2] = new NCursesMenuItem(); // empty item terminator

        paItems[0]->options_off(O_SELECTABLE);
        InitMenu(paItems, false, true);
        set_format(1, nItems);
    }
    ~SentContactRequest() {}
};

class ReceivedContactRequest : public NCursesMenu {
private:
    NCursesMenuItem ** paItems;
    enum { nItems = 3 };

public:
    ReceivedContactRequest(const std::string & username, int position) :
        NCursesMenu(1, username.length() + 2 + 7 * nItems + 2, 2 + position, 2),
        paItems(0)
    {
        paItems    = new NCursesMenuItem *[1 + nItems];
        paItems[0] = new NCursesMenuItem(username.c_str());
        paItems[1] = new RemoveItem("Accept ");
        paItems[2] = new RemoveItem("Reject ");
        paItems[3] = new NCursesMenuItem(); // empty item terminator

        paItems[0]->options_off(O_SELECTABLE);
        InitMenu(paItems, false, true);
        set_format(1, nItems);
    }
};

#endif // CONTACTMENU_H
