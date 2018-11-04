#include "popup.h"
#include <vector>

bool AcceptItem::action()
{
    popup->setResponse(0);
    return true;
}

bool RejectItem::action()
{
    popup->setResponse(1);
    return true;
}

bool DeferItem::action()
{
    popup->setResponse(2);
    return true;
}

Popup::Popup(const std::string & username, int width) :
    NCursesMenu(1, width, (lines() + 3) / 2, (cols() - width) / 2),
    response(0),
    panel(0),
    items(0)
{
    std::vector<std::string> buttonLabels = {"Accept", "Reject", "Defer"};
    int                      nItems       = buttonLabels.size();
    int                      buttonLength = width / nItems;
    for (auto && label : buttonLabels)
        for (int i = label.size(); i < buttonLength; i++) label += " ";

    items    = new NCursesMenuItem *[nItems + 1];
    items[0] = new AcceptItem(this, buttonLabels[0]);
    items[1] = new RejectItem(this, buttonLabels[1]);
    items[2] = new DeferItem(this, buttonLabels[2]);
    items[3] = new NCursesMenuItem();

    InitMenu(items, false, true);
    set_format(1, 3);

    panel = new NCursesPanel(3, width, (lines() - 3) / 2, (cols() - width) / 2);
    panel->bkgd(' ' | COLOR_PAIR(4));
    panel->addstr(1, 1, "User");
    panel->attron(A_BOLD);
    panel->addstr(1, 6, username.c_str());
    panel->attroff(A_BOLD);
    panel->addstr(1, 6 + username.length() + 1, "asked to establish a connection.");
    panel->show();
}

Popup::~Popup()
{
    panel->hide();
    refresh();
    delete panel;
}
