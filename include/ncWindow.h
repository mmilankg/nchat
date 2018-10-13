#ifndef NCWINDOW_H
#define NCWINDOW_H

#include "socket.h"
#include "topMenu.h"
#include "user.h"

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
    Socket *       pSocket;
    TopMenu *      pTopMenu;
    NCursesPanel * pBackground;
    NCursesPanel * pContactsPanel;
    NCursesPanel * pHistoryPanel;
    NCursesPanel * pMessagePanel;
    // vector of contacts
    std::vector<Contact> contacts;
    // vector of pointers to contact entries in the panel
    std::vector<NCursesMenu *> vpContactMenus;

public:
    NCWindow(Socket * pSock);
    ~NCWindow()
    {
        delete pBackground;
        delete pTopMenu;
        delete pContactsPanel;
        delete pHistoryPanel;
        delete pMessagePanel;
        for (std::vector<NCursesMenu *>::iterator it = vpContactMenus.begin(); it != vpContactMenus.end(); it++) {
            (*it)->unpost();
            (*it)->hide();
            (*it)->refresh();
            delete *it;
        }
    }

    void run();

private:
    void addContact(const std::string & username, int origin);
};

#endif // NCWINDOW_H
