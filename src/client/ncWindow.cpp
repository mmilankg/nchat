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

  /* DBG: testing the select call in the client program */
  /*
   * Prepare for the select() call that should intercept socket and
   * keyboard events.
   */
  fd_set socketDescriptors;
  FD_ZERO(&socketDescriptors);
  int stdinFD = fileno(stdin);
  int socketFD = pSocket->getSfd();
  FD_SET(stdinFD, &socketDescriptors);
  FD_SET(socketFD, &socketDescriptors);
  int nSockets = socketFD;
  select(nSockets + 1, &socketDescriptors, 0, 0, 0);
  enum PanelSelection { eTopMenu, eContacts, eHistory, eMessage };
  PanelSelection panelSelection = eTopMenu;
  sleep(2);
  /*
  bool logout = false;
  bool quit = false;
  while (!logout && !quit) {
    pTopMenu->operator()();
    logout = pTopMenu->getLogoutStatus();
    quit = pTopMenu->getQuitStatus();
  }
  */

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
