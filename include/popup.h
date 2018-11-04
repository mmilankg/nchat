#ifndef POPUP_H
#define POPUP_H

#include <cursesm.h>
#include <cursesp.h>
#include <string>

class AcceptItem : public NCursesMenuItem {
public:
    AcceptItem(const std::string & title) : NCursesMenuItem(title.c_str()) {}

    bool action() { return true; }
};

class RejectItem : public NCursesMenuItem {
public:
    RejectItem(const std::string & title) : NCursesMenuItem(title.c_str()) {}

    bool action() { return true; }
};

class DeferItem : public NCursesMenuItem {
public:
    DeferItem(const std::string & title) : NCursesMenuItem(title.c_str()) {}

    bool action() { return true; }
};

class Popup : public NCursesMenu {
    NCursesPanel *     panel;
    NCursesMenuItem ** items;

public:
    Popup(const std::string & username, int width = 50);
    virtual ~Popup();
};

#endif // POPUP_H
