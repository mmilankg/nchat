#ifndef TOPMENU_H
#define TOPMENU_H

#include <cursesm.h>

class ContactsItem : public NCursesMenuItem {
  public:
  ContactsItem(const char* pTitle) : NCursesMenuItem(pTitle) { }
};

class AccountItem : public NCursesMenuItem {
  public:
  AccountItem(const char* pTitle) : NCursesMenuItem(pTitle) { }
};

class TopMenu : public NCursesMenu {
  private:
  NCursesMenuItem** paItems;
  enum { nItems = 2 };

  public:
  TopMenu() : NCursesMenu(1, 10 * nItems + 2, 0, 0), paItems(0) {
    paItems = new NCursesMenuItem*[1 + nItems];
    paItems[0] = new ContactsItem("Contacts  ");
    paItems[1] = new AccountItem("Account  ");
    paItems[2] = new NCursesMenuItem();	// empty item terminator

    InitMenu(paItems, false, true);
    set_format(1, nItems);
  }
};

#endif	// TOPMENU_H
