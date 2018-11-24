#include "contactMenu.h"
#include "keys.h"
#include "ncWindow.h"
#include "popup.h"
#include "user.h"
#include <cursesp.h>
#include <fstream>
#include <vector>

NCWindow::NCWindow(Socket * pSock, const std::string & uname) : pSocket(pSock), username(uname)
{
    /* Create the background panel and split it into three areas. */
    pBackground = new NCursesPanel();
    pBackground->box();
    pBackground->move(1, pBackground->width() / 3);
    pBackground->vline(pBackground->height() - 2);
    pBackground->move(2 * pBackground->height() / 3, pBackground->width() / 3 + 1);
    pBackground->hline(2 * pBackground->width() / 3 - 1);
    pBackground->addch(0, pBackground->width() / 3, ACS_TTEE);
    pBackground->addch(pBackground->height() - 1, pBackground->width() / 3, ACS_BTEE);
    pBackground->addch(2 * pBackground->height() / 3, pBackground->width() / 3, ACS_LTEE);
    pBackground->addch(2 * pBackground->height() / 3, pBackground->width() - 1, ACS_RTEE);

    /* Create the top menu. */
    pTopMenu = new TopMenu(this, pSocket);

    /* Create the contacts panel. */
    pContactsPanel = new NCursesPanel(pBackground->height() - 2, pBackground->width() / 3 - 1, 1, 1);
    pContactsPanel->frame("Contacts");

    /* Create chat history panel. */
    pHistoryPanel = new NCursesPanel(2 * pBackground->height() / 3 - 1,
                                     2 * pBackground->width() / 3 - 1,
                                     1,
                                     pBackground->width() / 3 + 1);
    pHistoryPanel->frame("History");
    /* Create the display area of the history panel. */
    pInnerHistoryPanel = new NCursesPanel(pHistoryPanel->height() - 2,
                                          pHistoryPanel->width() - 2,
                                          2,
                                          pBackground->width() / 3 + 2);

    /* Create current message panel. */
    pMessagePanel = new NCursesPanel(pBackground->height() / 3 - 1,
                                     2 * pBackground->width() / 3 - 1,
                                     2 * pBackground->height() / 3 + 1,
                                     pBackground->width() / 3 + 1);
    pMessagePanel->frame("Message");
    /* Create the display area of the message panel. */
    pInnerMessagePanel = new NCursesPanel(pMessagePanel->height() - 2,
                                          pMessagePanel->width() - 2,
                                          2 * pBackground->height() / 3 + 2,
                                          pBackground->width() / 3 + 2);

    /* Create menu for each user in the list of contacts. */
    for (std::vector<Contact>::iterator it = contacts.begin(); it != contacts.end(); it++) {
        ContactMenu * pContactMenu = new ContactMenu(pSocket, it->getUsername(), vpContactMenus.size());
        vpContactMenus.push_back(pContactMenu);
    }
}

void NCWindow::run()
{

    pBackground->refresh();
    pTopMenu->post();
    pTopMenu->show();
    pTopMenu->refresh();

    fd_set fileDescriptors;
    int    stdinFD  = fileno(stdin);
    int    socketFD = pSocket->getSfd();
    int    nSockets = socketFD;
    // for panel selection
    enum PanelSelection { eTopMenu, eContacts, eHistory, eMessage };
    PanelSelection panelSelection = eTopMenu;
    // contact selection in the contacts menu
    /* DBG: Selected contact should be updated by processing up/down arrow keys when the contacts panel is selected, but
     * this hasn't been implemented yet.  So far, only the first contact will be used. */
    int selectedContact = 0;

    /* DBG: Set up buffer for receiving messages from the server. */
    std::vector<char> buffer;
    bool              logout = false;
    bool              exit   = false;
    while (!logout && !exit) {
        // Prepare for the select() call that should intercept socket and keyboard events.
        FD_ZERO(&fileDescriptors);
        FD_SET(stdinFD, &fileDescriptors);
        FD_SET(socketFD, &fileDescriptors);
        select(nSockets + 1, &fileDescriptors, 0, 0, 0);
        if (FD_ISSET(stdinFD, &fileDescriptors)) {
            int key = getch();
            // Use the horizontal tab key to switch between panels.
            if (key == KEY_HTAB) {
                panelSelection = PanelSelection(panelSelection + 1);
                if (panelSelection > eMessage) panelSelection = eTopMenu;
                switch (panelSelection) {
                case eTopMenu: {
                    pMessagePanel->frame("Message");
                    break;
                }
                case eContacts: {
                    pContactsPanel->boldframe("Contacts");
                    break;
                }
                case eHistory: {
                    pContactsPanel->frame("Contacts");
                    pHistoryPanel->boldframe("History");
                    break;
                }
                case eMessage: {
                    pHistoryPanel->frame("History");
                    pMessagePanel->boldframe("Message");
                    break;
                }
                }
            }
            else {
                switch (panelSelection) {
                case eTopMenu: {
                    pTopMenu->handleKey(key);
                    logout = pTopMenu->getLogoutStatus();
                    exit   = pTopMenu->getExitStatus();
                    break;
                }
                case eContacts: {
                    assert(selectedContact < vpContactMenus.size());
                    /* DBG: Is downcasting acceptable? */
                    ((ContactMenu *)vpContactMenus[selectedContact])->setActivated(0);
                    if (vpContactMenus.size() > 0) ((ContactMenu *)vpContactMenus[selectedContact])->handleKey(key);
                    /* DBG: process only text message requests for the moment. */
                    if (((ContactMenu *)vpContactMenus[selectedContact])->getActivated() == 1) {
                        /* Switch focus to message panel. */
                        pContactsPanel->frame("Contacts");
                        pMessagePanel->boldframe("Message");
                        panelSelection                    = eMessage;
                        std::string              username = (*vpContactMenus[selectedContact])[0]->name();
                        std::vector<std::string> contents;
                        contents.push_back(username);
                        std::string message;
                        pInnerMessagePanel->move(1, 1);
                        int character;
                        int x, y;
                        while ((character = pInnerMessagePanel->getch()) != '\n') {
                            if (character == KEY_BACKSPACE) {
                                message.pop_back();
                                pInnerMessagePanel->getyx(y, x);
                                pInnerMessagePanel->move(y, x - 1);
                                pInnerMessagePanel->addch(' ');
                                pInnerMessagePanel->move(y, x - 1);
                            }
                            else {
                                message.push_back(character);
                                pInnerMessagePanel->addch(character);
                            }
                        }
                        contents.push_back(message);
                        pSocket->send(mText, contents);
                        /* Transfer the message into the history panel (right-align). */
                        pInnerMessagePanel->move(1, 1);
                        for (int i = 0; i < message.length(); i++) pInnerMessagePanel->addch(' ');
                        pInnerMessagePanel->refresh();
                        pInnerMessagePanel->move(1, 1);
                        int x0, y0;
                        pInnerHistoryPanel->getyx(y0, x0);
                        x0 = pInnerHistoryPanel->width() - message.length() - 6;
                        pInnerHistoryPanel->move(y0 + 1, x0);
                        pInnerHistoryPanel->addstr("me > ");
                        pInnerHistoryPanel->addstr(message.c_str());
                        pInnerHistoryPanel->refresh();
                    }
                    break;
                }
                }
            }
            pBackground->refresh();
        }
        if (FD_ISSET(socketFD, &fileDescriptors)) {
            /* message processing similar to the server process */
            int         messageLength;
            MessageType messageType;
            pSocket->recv(messageLength);
            pSocket->recv(messageType);
            int contentLength = messageLength - sizeof(messageLength) - sizeof(messageType);
            pSocket->recv(buffer, contentLength);

            // Process the message.
            switch (messageType) {
            // server response to a sent request for establishing a contact with another user
            case mFindUser: {
                int serverResponse;
                std::memcpy(&serverResponse, buffer.data(), sizeof(serverResponse));
                std::string    requestedUsername = buffer.data() + sizeof(serverResponse);
                FindUserDialog findUserResponse(serverResponse, requestedUsername);
                findUserResponse.run();

                /* Add the contact entry to the contacts panel. */
                if (serverResponse == 0) addContact(requestedUsername, cSentContact);
                break;
            }
            case mContactRequest: {
                std::string sendingUsername = buffer.data();
                // user's response to the contact request:
                // 0: accept
                // 1: reject
                // 2: defer
                Popup contactRequest(sendingUsername);
                contactRequest();
                int response = contactRequest.getResponse();

                handleContactRequest(sendingUsername, response);

                break;
            }
            case mEstablishedContact: {
                /*
                 * In the first instance, receive data based on what is sent through User::transmit() function.  A
                 * better implementation would create a new object of the User class (or, possibly even better, of a
                 * Contact class), place it into a vector and use its member function to receive the data from the
                 * server.
                 */
                int userID;
                std::memcpy(&userID, buffer.data(), sizeof(userID));
                std::string username = buffer.data() + sizeof(userID);
                std::string name     = buffer.data() + sizeof(userID) + username.length() + 1;
                int         status;
                std::memcpy(&userID,
                            buffer.data() + sizeof(userID) + username.length() + 1 + name.length() + 1,
                            sizeof(status));
                addContact(username, cEstablishedContact);
                break;
            }
            case mText: {
                /*
                 * In the first instance, just receive the message and display it in the message history.  This means
                 * that messages from all contacts will be displayed there in this implementation.  A better solution
                 * would be to display a contact-specific message history, but that requires storing contact-specific
                 * messages in a buffer or a file, where they are stored for displaying when that contact is selected.
                 */
                std::string senderUsername = buffer.data();
                std::string message        = buffer.data() + senderUsername.length() + 1;
                displayMessage(senderUsername, message);
                break;
            }
            case mCall: {
                // Handle a call request in a similar way to how contact requests are handled.
                std::string initiatorUsername = buffer.data();
                // user's response to initiated call request:
                // 0: accept
                // 1: reject
                CallPopup callRequest(initiatorUsername);
                callRequest();
                int response = callRequest.getResponse();
                break;
            }
            }
        }
    }

    pTopMenu->unpost();
    pTopMenu->hide();
    pTopMenu->refresh();
    pBackground->clear();
    pBackground->refresh();
}

void NCWindow::addContact(const std::string & username, ContactType contactType)
{
    /* Add an entry into the contacts panel. */
    NCursesMenu * pContact;
    if (contactType == cEstablishedContact)
        pContact = new ContactMenu(pSocket, username, vpContactMenus.size());
    else if (contactType == cSentContact)
        pContact = new SentContactRequest(username, vpContactMenus.size());
    else
        pContact = new ReceivedContactRequest(username, vpContactMenus.size());
    vpContactMenus.push_back(pContact);
    pContact->post();
    pContact->show();
    pContact->refresh();
    pBackground->refresh();
}

void NCWindow::handleContactRequest(const std::string & username, int response)
{
    switch (response) {
    case (0): {
        NCursesMenu * pContact = new ContactMenu(pSocket, username, vpContactMenus.size());
        vpContactMenus.push_back(pContact);
        pContact->post();
        pContact->show();
        pContact->refresh();
        pBackground->refresh();
        break;
    }
    case (1): {
        break;
    }
    case (2): {
        addContact(username, cReceivedContact);
        break;
    }
    }

    /* If user hasn't deferred the decision (response < 2), send their response to the contact request to server. */
    if (response < 2) {
        int messageLength = sizeof(messageLength);
        messageLength += sizeof(mContactRequest);
        int messageContentLength = sizeof(int) + username.length() + 1;
        messageLength += messageContentLength;
        std::vector<char> messageContent;
        messageContent.resize(messageContentLength);
        std::memcpy(messageContent.data(), &response, sizeof(response));
        std::memcpy(messageContent.data() + sizeof(response), username.c_str(), username.length());
        pSocket->send(messageLength);
        pSocket->send(mContactRequest);
        pSocket->send(messageContent);
    }
}

void NCWindow::displayMessage(const std::string & senderUsername, const std::string & message)
{
    int x0, y0;
    pInnerHistoryPanel->getyx(y0, x0);
    pInnerHistoryPanel->move(y0 + 1, 1);
    pInnerHistoryPanel->addstr(senderUsername.c_str());
    pInnerHistoryPanel->addstr(" > ");
    pInnerHistoryPanel->addstr(message.c_str());
    pInnerHistoryPanel->refresh();
}
