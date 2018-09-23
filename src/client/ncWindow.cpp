#include "contactMenu.h"
#include "ncWindow.h"
#include "topMenu.h"
#include "user.h"
#include <cursesp.h>
#include <fstream>
#include <vector>

void NCWindow::run() {
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
  TopMenu topMenu;

  // vector of contacts
  std::vector<Contact> contacts;

  /* Create menu for each user in the list of contacts. */
  std::vector<ContactMenu*> vpContactMenus;
  for (std::vector<Contact>::iterator it = contacts.begin(); it != contacts.end(); it++) {
    ContactMenu* pContactMenu = new ContactMenu(it->getUsername());
    vpContactMenus.push_back(pContactMenu);
  }

  topMenu();
}
