#include "contactMenu.h"
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
    pContactsPanel->boldframe("Contacts");

    /* Create chat history panel. */
    pHistoryPanel = new NCursesPanel(2 * pBackground->height() / 3 - 1,
                                     2 * pBackground->width() / 3 - 1,
                                     1,
                                     pBackground->width() / 3 + 1);
    pHistoryPanel->boldframe("History");

    /* Create current message panel. */
    pMessagePanel = new NCursesPanel(pBackground->height() / 3 - 1,
                                     2 * pBackground->width() / 3 - 1,
                                     2 * pBackground->height() / 3 + 1,
                                     pBackground->width() / 3 + 1);
    pMessagePanel->boldframe("Message");

    /* Create menu for each user in the list of contacts. */
    for (std::vector<Contact>::iterator it = contacts.begin(); it != contacts.end(); it++) {
        ContactMenu * pContactMenu = new ContactMenu(it->getUsername(), vpContactMenus.size());
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
    enum PanelSelection { eTopMenu, eContacts, eHistory, eMessage };
    PanelSelection panelSelection = eTopMenu;

    /* DBG: Set up buffer for receiving messages from the server. */
    char * buffer = new char[1024]();
    bool   logout = false;
    bool   exit   = false;
    while (!logout && !exit) {
        /*
         * Prepare for the select() call that should intercept socket and
         * keyboard events.
         */
        FD_ZERO(&fileDescriptors);
        FD_SET(stdinFD, &fileDescriptors);
        FD_SET(socketFD, &fileDescriptors);
        select(nSockets + 1, &fileDescriptors, 0, 0, 0);
        if (FD_ISSET(stdinFD, &fileDescriptors)) {
            int key = getch();
            pTopMenu->handleKey(key);
            logout = pTopMenu->getLogoutStatus();
            exit   = pTopMenu->getExitStatus();
        }
        if (FD_ISSET(socketFD, &fileDescriptors)) {
            /* message processing similar to the server process */
            int         messageLength;
            MessageType messageType;
            pSocket->recv(messageLength);
            pSocket->recv(messageType);
            pSocket->recv(buffer);
            int contentLength = messageLength - sizeof(messageLength) - sizeof(messageType);

            // Process the message.
            switch (messageType) {
            // server response to a sent request for establishing a contact with another user
            case mFindUser: {
                int serverResponse = *buffer;
                buffer += sizeof(serverResponse);
                std::string    requestedUsername = buffer;
                FindUserDialog findUserResponse(serverResponse, requestedUsername);
                findUserResponse.run();

                /* Add the contact entry to the contacts panel. */
                if (serverResponse == 0) addContact(requestedUsername, 1);
                break;
            }
            case mContactRequest: {
                std::string sendingUsername = buffer;
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
            }
        }
    }

    pTopMenu->unpost();
    pTopMenu->hide();
    pTopMenu->refresh();
    pBackground->clear();
    pBackground->refresh();
}

void NCWindow::addContact(const std::string & username, int origin)
{
    /*
     * Add an entry into the contacts panel.  If origin is 0, this will be an entry for a user who sent the contact
     * request.  If origin is 1, it will be for a user who receives it.
     *
     * DBG: Is this considered a bad practice?  Would an enumerated type be better to use for origin?
     */
    NCursesMenu * pContact;
    if (origin == 0)
        pContact = new ReceivedContactRequest(username, vpContactMenus.size());
    else
        pContact = new SentContactRequest(username, vpContactMenus.size());
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
        NCursesMenu * pContact = new ContactMenu(username, vpContactMenus.size());
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
        addContact(username, 0);
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
