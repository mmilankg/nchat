#include "topMenu.h"

FindContactDialog::FindContactDialog(Socket * pSocket, int h, int w, int y, int x) : Dialog(h, w, y, x)
{
    paFormFields    = new NCursesFormField *[3];
    paFormFields[0] = new LabelField("Username", 1, 2);
    paFormFields[1] = new NCursesFormField(1, w - 6, 2, 2);
    paFormFields[2] = new NCursesFormField();

    InitForm(paFormFields, true, true);

    paMenuItems    = new NCursesMenuItem *[4];
    paMenuItems[0] = new FindContactOKItem(this, pSocket);
    paMenuItems[1] = new FindContactCancelItem();
    paMenuItems[2] = new NCursesMenuItem(" ");
    paMenuItems[3] = new NCursesMenuItem();

    InitMenu(paMenuItems, false, true);
    set_format(1, 3);
}

bool FindContactOKItem::action()
{
    NCursesFormField * pUsernameFormField = pParentPanel->getField(1);
    std::string        requestedUsername  = pUsernameFormField->value();
    requestedUsername                     = requestedUsername.substr(0, requestedUsername.find_last_not_of(" ") + 1);

    // Send contact request to server.
    pSocket->send(mFindUser, requestedUsername);

    return true;
}

/* DBG: There should be a better place for this, especially because it
 * also appears in dialog.cpp! */
static const int CMD_QUIT   = MAX_COMMAND + 1;
static const int CMD_ACTION = MAX_COMMAND + 2;

void TopMenu::handleKey(int key)
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
