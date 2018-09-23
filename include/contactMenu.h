#ifndef CONTACTMENU_H
#define CONTACTMENU_H

#include <cursesm.h>
#include <string>

class TextItem : public NCursesMenuItem {
  public:
  TextItem(const char* pTitle) : NCursesMenuItem(pTitle) { }
};

class CallItem : public NCursesMenuItem {
  public:
  CallItem(const char* pTitle) : NCursesMenuItem(pTitle) { }
};

class ContactMenu : public NCursesMenu {
  private:
  NCursesMenuItem** paItems;
  enum { nItems = 3 };

  public:
  ContactMenu(const std::string& username) :
    NCursesMenu(1, username.length() + 2 + 7 * nItems + 2, 0, 0), paItems(0)
  {
    paItems = new NCursesMenuItem*[1 + nItems];
    paItems[0] = new NCursesMenuItem(username.c_str());
    paItems[1] = new TextItem("Text   ");
    paItems[2] = new CallItem("Call   ");
    paItems[3] = new NCursesMenuItem();	// empty item terminator

    paItems[0]->options_off(O_SELECTABLE);
    InitMenu(paItems, false, true);
    set_format(1, nItems);
  }
};

#endif	// CONTACTMENU_H
