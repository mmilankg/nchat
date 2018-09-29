#include "topMenu.h"

FindContactDialog::FindContactDialog(Socket* pSocket, int h, int w, int y, int x) :
  Dialog(h, w, y, x)
{
  paFormFields = new NCursesFormField*[3];
  paFormFields[0] = new LabelField("Username", 4, 2);
  paFormFields[1] = new NCursesFormField(1, w - 6, 5, 2);
  paFormFields[2] = new NCursesFormField();

  InitForm(paFormFields, true, true);

  paMenuItems = new NCursesMenuItem*[4];
  paMenuItems[0] = new FindContactOKItem(this, pSocket);
  paMenuItems[1] = new FindContactCancelItem();
  paMenuItems[2] = new NCursesMenuItem(" ");
  paMenuItems[3] = new NCursesMenuItem();

  InitMenu(paMenuItems, false, true);
  set_format(1, 3);
}

bool FindContactOKItem::action() {
  NCursesFormField* pUsernameFormField = pParentPanel->getField(1);
  std::string username = pUsernameFormField->value();
  username = username.substr(0, username.find_last_not_of(" ") + 1);

  // Send contact request to server.
  pSocket->send(mFindUser, username);
}
