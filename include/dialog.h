#ifndef DIALOG_H
#define DIALOG_H

#include "keys.h"
#include <vector>
#include <string>
#include <cursesm.h>
#include <cursesf.h>

class DialogItem;

class Dialog : public NCursesForm, public NCursesMenu {
    int dialogHeight, dialogWidth;
    int dialogY, dialogX;

protected:
    bool                       inForm, inMenu;
    NCursesFormField **        paFormFields;
    NCursesMenuItem **         paMenuItems;
    Alphanumeric_Field *       pAlphaNumericFieldType;
    Regular_Expression_Field * pRegularExpressionFieldType;

    Dialog() :
        NCursesForm(0, 0),
        NCursesMenu(0, 0),
        paFormFields(NULL),
        paMenuItems(NULL),
        pAlphaNumericFieldType(NULL),
        pRegularExpressionFieldType(NULL)
    {
    }

public:
    Dialog(int height, int width, int y0, int x0);
    Dialog(int                              height,
           int                              width,
           int                              y0,
           int                              x0,
           const std::vector<std::string> & fieldLabels,
           const std::vector<int> &         fieldTypes,
           const std::vector<std::string> & buttonLabels);

    virtual ~Dialog()
    {
        delete pAlphaNumericFieldType;
        delete pRegularExpressionFieldType;
    }

    virtual void run();

    int virtualize(int key);

    int driver(int command);

    int getKey();

    virtual NCursesFormField * getField(int index) const { return paFormFields[index]; }
};

class DialogItem : public NCursesMenuItem {
    Dialog * pParentDialog;

public:
    DialogItem(const char * s, Dialog * pDialog) : NCursesMenuItem(s) { pParentDialog = pDialog; }
};

// field item for static labels (adapted from ncurses demo.cc)
class LabelField : public NCursesFormField {
public:
    LabelField(const std::string & title, int row, int column) : NCursesFormField(1, title.length(), row, column)
    {
        set_value(title.c_str());
        options_off(O_EDIT | O_ACTIVE);
    }
};

class OKItem : public NCursesMenuItem {
public:
    OKItem() : NCursesMenuItem("                     OK                     "){};
    bool action() { return true; }
};

class FindUserDialog : public Dialog {
public:
    FindUserDialog(int serverResponse, const std::string & username, int h = 3, int w = 45, int y = 0, int x = 0) :
        Dialog(h, w, y, x)
    {
        paFormFields        = new NCursesFormField *[2];
        std::string message = "User " + username + " has ";
        if (serverResponse == 1) message += "not ";
        message += "been found.";
        if (serverResponse == 2) message = "Self-contact is not allowed.";
        paFormFields[0] = new LabelField(message, 0, 1);
        paFormFields[1] = new NCursesFormField();
        InitForm(paFormFields, true, true);

        paMenuItems    = new NCursesMenuItem *[2];
        paMenuItems[0] = new OKItem();
        paMenuItems[1] = new NCursesMenuItem();
        InitMenu(paMenuItems, false, true);

        inForm = false;
        inMenu = true;
    }
};

#endif // DIALOG_H
