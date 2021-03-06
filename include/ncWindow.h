#ifndef NCWINDOW_H
#define NCWINDOW_H

#include "contactMenu.h"
#include "socket.h"
#include "topMenu.h"
#include "user.h"

/*
 * This class will represent the main window presented to the user when they log in or after the successful sign up
 * process.
 *
 * The design of the class is inspired by the SillyDemo class of the ncurses C++ demo program, which doesn't have any
 * members, but other ncurses elements are started when the object of this class is created and its run() function
 * invoked.
 */
class NCWindow {
    Socket *       pSocket;
    TopMenu *      pTopMenu;
    NCursesPanel * pBackground;
    NCursesPanel * pContactsPanel;
    NCursesPanel * pHistoryPanel;
    // NCursesPad *   pHistoryPanel;
    NCursesPanel * pInnerHistoryPanel;
    NCursesPanel * pInnerMessagePanel;
    NCursesPanel * pMessagePanel;
    // username of the user that logged in
    std::string username;
    // vector of contacts
    std::vector<Contact> contacts;
    // vector of pointers to contact entries in the panel
    std::vector<NCursesMenu *> vpContactMenus;

public:
    NCWindow(Socket * pSock, const std::string & uname);
    ~NCWindow()
    {
        delete pBackground;
        delete pTopMenu;
        delete pContactsPanel;
        delete pHistoryPanel;
        delete pInnerHistoryPanel;
        delete pMessagePanel;
        delete pInnerMessagePanel;
        for (auto pContactMenu : vpContactMenus) {
            pContactMenu->unpost();
            pContactMenu->hide();
            pContactMenu->refresh();
            delete pContactMenu;
        }
    }

    void          run();
    std::string & getUsername() { return username; }

private:
    void addContact(const std::string & username, ContactType contactType);
    void handleContactRequest(const std::string & username, int response);
    void handleCallRequest(const std::string & username, int response);
    void displayMessage(const std::string & senderUsername, const std::string & message);
};

#endif // NCWINDOW_H
