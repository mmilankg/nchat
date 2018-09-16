#include "loginDialog.h"

LoginDialog::LoginDialog(const Socket* pSock, int h, int w, int y, int x) :
  pSocket(pSock),
  Dialog(h, w, y, x)
{
  paFormFields = new NCursesFormField*[5];
  paFormFields[0] = new LabelField("Username", 1, 2);
  paFormFields[1] = new NCursesFormField(1, w - 6, 2, 2);
  paFormFields[2] = new LabelField("Password", 4, 2);
  paFormFields[3] = new NCursesFormField(1, w - 6, 5, 2);
  paFormFields[4] = new NCursesFormField();
  pAlphaNumericFieldType = new Alphanumeric_Field(1);
  pRegularExpressionFieldType = new Regular_Expression_Field("[A-Za-z0-9]*");
  paFormFields[0]->set_fieldtype(*pAlphaNumericFieldType);
  paFormFields[1]->set_fieldtype(*pAlphaNumericFieldType);
  paFormFields[2]->set_fieldtype(*pAlphaNumericFieldType);
  paFormFields[3]->set_fieldtype(*pRegularExpressionFieldType);
  /*
   * DBG: Replace characters with ' ' in the password field!
   * Ideally a new class for field type would handle this better.
   */
  paFormFields[3]->options_off(O_PUBLIC);

  InitForm(paFormFields, true, true);

  paMenuItems = new NCursesMenuItem*[5];
  paMenuItems[0] = new LoginItem(this, pSocket);
  paMenuItems[1] = new SignupItem(pSock);
  paMenuItems[2] = new QuitItem(pSock);
  /*
   * DBG: The following fake item doesn't get highlighted when
   * it's selected, which leaves the desired impression that no
   * item is selected when the cursor is in the form part of the
   * panel.  A better solution would be to create a menu in
   * which all items can be deselected.
   */
  paMenuItems[3] = new NCursesMenuItem(" ");
  paMenuItems[4] = new NCursesMenuItem();

  InitMenu(paMenuItems, false, true);
  set_format(1, 4);
}

bool LoginItem::action() {
  /*
   * The field in which the username is stored is with the index 1.  Its label
   * is at index 0.
   */
  const NCursesFormField* pUsernameFormField = pParentPanel->getField(1);
  // Similarly, password is at field with index 3 (its label at index 2).
  const NCursesFormField* pPasswordFormField = pParentPanel->getField(3);
  std::string username = pUsernameFormField->value();
  std::string password = pPasswordFormField->value();
  username = username.substr(0, username.find_last_not_of(" ") + 1);
  password = password.substr(0, password.find_last_not_of(" ") + 1);

  // Send the username and password to the server (similar to SignupOKItem::action()).
  int serverResponse;
  pSocket->send(cLogin);
  pSocket->recv(serverResponse);
  pSocket->send(username);
  pSocket->recv(serverResponse);
  pSocket->send(password);
  pSocket->recv(serverResponse);

  if (serverResponse == 0) {
    // Start the main working window if the user is logged in.
    /* DBG: start the signupDialog just for testing! */
    SignupDialog signupDialog(pSocket);
    signupDialog.run();
  }

  return false;
}
