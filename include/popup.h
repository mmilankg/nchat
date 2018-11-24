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

/*
 * The following code implements a CallPopup class which is used for displaying a popup window when another user
 * initiates a voice call.  The problem is that it's very similar to the Popup class above, but not the same as it
 * doesn't have the Defer button, and the Accept and Reject buttons should have different action() functions.  It seems
 * logical that there is a better way to reuse the code rather than just copying and modifying it.
 */

class CallPopup;

class AcceptCallItem : public NCursesMenuItem {
    CallPopup * callPopup;

public:
    AcceptCallItem(CallPopup * pCallPopup, const std::string & title) :
        callPopup(pCallPopup),
        NCursesMenuItem(title.c_str())
    {
    }

    bool action();
};

class RejectCallItem : public NCursesMenuItem {
    CallPopup * callPopup;

public:
    RejectCallItem(CallPopup * pCallPopup, const std::string & title) :
        callPopup(pCallPopup),
        NCursesMenuItem(title.c_str())
    {
    }

    bool action();
};

class CallPopup : public NCursesMenu {
    NCursesPanel *     panel;
    NCursesMenuItem ** items;
    int                response;

public:
    CallPopup(const std::string & username, int width = 50);
    virtual ~CallPopup();

    void setResponse(int resp) { response = resp; }
    int  getResponse() const { return response; }
};

#endif // POPUP_H
