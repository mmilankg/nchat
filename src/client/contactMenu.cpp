#include "contactMenu.h"
#include <cursesm.h>

bool TextItem::action()
{
    parent->setActivated(1);
}

/* DBG: There should be a better place for this, especially because it
 * also appears in dialog.cpp! */
static const int CMD_QUIT   = MAX_COMMAND + 1;
static const int CMD_ACTION = MAX_COMMAND + 2;

void ContactMenu::handleKey(int key)
/* DBG: Copied from TopMenu.  Perhaps a more effective solution would be to make a class that is derived from
 * NCursesMenu, implement the handleKey() function for the derived class, and then derive both the TopMenu and
 * ContactMenu classes from it. */
{
    // copied from cursesm.cc in ncurses-5.9 distribution
    int  err;
    bool bAction;
    int  driverCommand = virtualize(key);

    switch ((err = driver(driverCommand))) {
    case E_REQUEST_DENIED: On_Request_Denied(key); break;
    case E_NOT_SELECTABLE: On_Not_Selectable(key); break;
    case E_UNKNOWN_COMMAND:
        if (driverCommand == CMD_ACTION) {
            if (options() & O_ONEVALUE) {
                NCursesMenuItem * itm = current_item();
                assert(itm != 0);
                if (itm->options() & O_SELECTABLE) {
                    bAction = itm->action();
                    refresh();
                }
                else
                    On_Not_Selectable(key);
            }
            else {
                int n = count();
                for (int i = 0; i < n; i++) {
                    NCursesMenuItem * itm = operator[](i);
                    if (itm->value()) {
                        bAction |= itm->action();
                        refresh();
                    }
                }
            }
        }
        else
            On_Unknown_Command(key);
        break;
    case E_NO_MATCH: On_No_Match(key); break;
    case E_OK: break;
    default: OnError(err);
    }
    refresh();
}
