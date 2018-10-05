#include "contactMenu.h"
#include "ncWindow.h"
#include "topMenu.h"
#include "user.h"
#include <cursesp.h>
#include <fstream>
#include <vector>

void NCWindow::run() {
  // vector of contacts
  std::vector<Contact> contacts;
  /* Create menu for each user in the list of contacts. */
  std::vector<ContactMenu*> vpContactMenus;
  for (std::vector<Contact>::iterator it = contacts.begin(); it != contacts.end(); it++) {
    ContactMenu* pContactMenu = new ContactMenu(it->getUsername());
    vpContactMenus.push_back(pContactMenu);
  }

  /* Create the background panel and split it into three areas. */
  NCursesPanel background;
  background.box();
  background.move(1, background.width() / 3);
  background.vline(background.height() - 2);
  background.move(2 * background.height() / 3, background.width() / 3 + 1);
  background.hline(2 * background.width() / 3 - 1);
  background.addch(0, background.width() / 3, ACS_TTEE);
  background.addch(background.height() - 1, background.width() / 3, ACS_BTEE);
  background.addch(2 * background.height() / 3, background.width() / 3, ACS_LTEE);
  background.addch(2 * background.height() / 3, background.width() - 1, ACS_RTEE);

  /* Create the top menu. */
  TopMenu* pTopMenu = new TopMenu(pSocket);

  /* Create the contacts panel. */
  NCursesPanel* pContactsPanel =
    new NCursesPanel(background.height() - 2, background.width() / 3 - 1, 1, 1);
  pContactsPanel->boldframe("Contacts");

  /* Create chat history panel. */
  NCursesPanel* pHistoryPanel =
    new NCursesPanel(2 * background.height() / 3 - 1, 2 * background.width() / 3 - 1, 1, background.width() / 3 + 1);
  pHistoryPanel->boldframe("History");

  /* Create current message panel. */
  NCursesPanel* pMessagePanel =
    new NCursesPanel(background.height() / 3 - 1, 2 * background.width() / 3 - 1, 2 * background.height() / 3 + 1, background.width() / 3 + 1);
  pMessagePanel->boldframe("Message");

  background.refresh();
  pTopMenu->post();
  pTopMenu->show();
  pTopMenu->refresh();

  fd_set fileDescriptors;
  int stdinFD = fileno(stdin);
  int socketFD = pSocket->getSfd();
  int nSockets = socketFD;
  enum PanelSelection { eTopMenu, eContacts, eHistory, eMessage };
  PanelSelection panelSelection = eTopMenu;

  /* DBG: Set up buffer for receiving messages from the server. */
  char* buffer = new char[1024]();
  bool logout = false;
  bool quit = false;
  while (!logout && !quit) {
    /*
     * Prepare for the select() call that should intercept socket and
     * keyboard events.
     */
    FD_ZERO(&fileDescriptors);
    FD_SET(stdinFD, &fileDescriptors);
    FD_SET(socketFD, &fileDescriptors);
    select(nSockets + 1, &fileDescriptors, 0, 0, 0);
    if (FD_ISSET(stdinFD, &fileDescriptors)) {
      pTopMenu->handleKey();
      //pTopMenu->operator()();
      logout = pTopMenu->getLogoutStatus();
      quit = pTopMenu->getQuitStatus();
    }
    if (FD_ISSET(socketFD, &fileDescriptors)) {
      /* message processing similar to the server process */
      int messageLength;
      MessageType messageType;
      pSocket->recv(messageLength);
      pSocket->recv(messageType);
      pSocket->recv(buffer);
      int contentLength = messageLength - sizeof(messageLength) - sizeof(messageType);

      // Process the message.
      switch (messageType) {
	case mFindUser :
	  {
	    int serverResponse = *buffer;
	    buffer += sizeof(serverResponse);
	    std::string requestedUsername = buffer;
	    FindUserDialog findUserResponse(serverResponse, requestedUsername);
	    findUserResponse.run();
	  }
      }
    }
  }

  pTopMenu->unpost();
  pTopMenu->hide();
  pTopMenu->refresh();
  delete pTopMenu;
  delete pContactsPanel;
  delete pHistoryPanel;
  delete pMessagePanel;
  background.clear();
  background.refresh();
}
