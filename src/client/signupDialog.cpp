#include "ncWindow.h"
#include "signupDialog.h"

SignupDialog::SignupDialog(Socket * pSocket, int h, int w, int y, int x) : Dialog(h, w, y, x)
{
    paFormFields           = new NCursesFormField *[7];
    paFormFields[0]        = new LabelField("Name", 1, 2);
    paFormFields[1]        = new NCursesFormField(1, w - 6, 2, 2);
    paFormFields[2]        = new LabelField("Username", 4, 2);
    paFormFields[3]        = new NCursesFormField(1, w - 6, 5, 2);
    paFormFields[4]        = new LabelField("Password", 7, 2);
    paFormFields[5]        = new NCursesFormField(1, w - 6, 8, 2);
    paFormFields[6]        = new NCursesFormField();
    pAlphaNumericFieldType = new Alphanumeric_Field(1);
    // pRegularExpressionFieldType = new Regular_Expression_Field("[A-za-z -]*");
    // pRegularExpressionFieldType = new Regular_Expression_Field("[A-Za-z]");
    pRegularExpressionFieldType = new Regular_Expression_Field("^[0-9A-Za-z -]* *$");
    paFormFields[0]->set_fieldtype(*pAlphaNumericFieldType);
    paFormFields[1]->set_fieldtype(*pRegularExpressionFieldType);
    paFormFields[2]->set_fieldtype(*pAlphaNumericFieldType);
    paFormFields[3]->set_fieldtype(*pAlphaNumericFieldType);
    paFormFields[4]->set_fieldtype(*pAlphaNumericFieldType);
    paFormFields[5]->set_fieldtype(*pRegularExpressionFieldType);
    /*
     * DBG: Replace characters with ' ' in the password field!
     * Ideally a new class for field type would handle this better.
     */
    paFormFields[5]->options_off(O_PUBLIC);

    InitForm(paFormFields, true, true);

    paMenuItems    = new NCursesMenuItem *[4];
    paMenuItems[0] = new SignupOKItem(this, pSocket);
    paMenuItems[1] = new SignupCancelItem();
    /*
     * DBG: The following fake item doesn't get highlighted when
     * it's selected, which leaves the desired impression that no
     * item is selected when the cursor is in the form part of the
     * panel.  A better solution would be to create a menu in
     * which all items can be deselected.
     */
    paMenuItems[2] = new NCursesMenuItem(" ");
    paMenuItems[3] = new NCursesMenuItem();

    InitMenu(paMenuItems, false, true);
    set_format(1, 3);
}

bool SignupOKItem::action()
{
    NCursesFormField * pNameFormField     = pParentPanel->getField(1);
    NCursesFormField * pUsernameFormField = pParentPanel->getField(3);
    NCursesFormField * pPasswordFormField = pParentPanel->getField(5);
    std::string        name               = pNameFormField->value();
    std::string        username           = pUsernameFormField->value();
    std::string        password           = pPasswordFormField->value();
    name                                  = name.substr(0, name.find_last_not_of(" ") + 1);
    username                              = username.substr(0, username.find_last_not_of(" ") + 1);
    password                              = password.substr(0, password.find_last_not_of(" ") + 1);

    // Pack user details into a vector and send through the socket.
    std::vector<std::string> userDetails;
    userDetails.push_back(name);
    userDetails.push_back(username);
    userDetails.push_back(password);
    pSocket->send(mSignup, userDetails);

    // Get the server response.
    int         messageLength;
    MessageType messageType;
    int         serverResponse;
    // Prepare the server for the signup sequence.
    pSocket->recv(messageLength);
    pSocket->recv(messageType);
    pSocket->recv(serverResponse);

    if (serverResponse == 0) {
        // Start the main working window after the user has signed up.
        NCWindow ncWindow(pSocket, username);
        ncWindow.run();
    }
    return false;
}
