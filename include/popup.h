#ifndef POPUP_H
#define POPUP_H

#include <cursesm.h>
#include <cursesp.h>
#include <string>

class Popup;

class AcceptItem : public NCursesMenuItem {
    Popup * popup;

public:
    AcceptItem(Popup * pPopup, const std::string & title) : popup(pPopup), NCursesMenuItem(title.c_str()) {}

    bool action();
};

class RejectItem : public NCursesMenuItem {
    Popup * popup;

public:
    RejectItem(Popup * pPopup, const std::string & title) : popup(pPopup), NCursesMenuItem(title.c_str()) {}

    bool action();
};

class DeferItem : public NCursesMenuItem {
    Popup * popup;

public:
    DeferItem(Popup * pPopup, const std::string & title) : popup(pPopup), NCursesMenuItem(title.c_str()) {}

    bool action();
};

class Popup : public NCursesMenu {
    NCursesPanel *     panel;
    NCursesMenuItem ** items;
    int                response;

public:
    Popup(const std::string & username, int width = 50);
    virtual ~Popup();

    void setResponse(int resp) { response = resp; }
    int  getResponse() const { return response; }
};

#endif // POPUP_H
