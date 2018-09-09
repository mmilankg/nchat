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
  paMenuItems[0] = new LoginItem(this);
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
