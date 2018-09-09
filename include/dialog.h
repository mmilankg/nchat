#ifndef DIALOG_H
#define DIALOG_H

#include <vector>
#include <string>
#include <cursesm.h>
#include <cursesf.h>

#ifndef CTRL
#define CTRL(x) ((x) & 0x1f)
#endif	// CTRL
// horizontal tab key
#ifndef KEY_HTAB
#define KEY_HTAB 9
#endif	// KEY_HTAB
// login panel commands
#define DIALOG_PREV (KEY_MAX + 1)
#define DIALOG_NEXT (KEY_MAX + 2)
#define DIALOG_QUIT (KEY_MAX + 3)
#define DIALOG_ENTER (KEY_MAX + 4)

class DialogItem;

class Dialog : public NCursesForm, public NCursesMenu {
  int dialogHeight, dialogWidth;
  int dialogY, dialogX;

  protected:
  bool inForm, inMenu;
  NCursesFormField** paFormFields;
  NCursesMenuItem** paMenuItems;
  Alphanumeric_Field* pAlphaNumericFieldType;
  Regular_Expression_Field* pRegularExpressionFieldType;

  Dialog() : NCursesForm(0, 0), NCursesMenu(0, 0), paFormFields(NULL), paMenuItems(NULL),
  pAlphaNumericFieldType(NULL), pRegularExpressionFieldType(NULL)
  { }
  public:
  Dialog(int height, int width, int y0, int x0);
  Dialog(int height, int width, int y0, int x0,
      const std::vector<std::string>& fieldLabels,
      const std::vector<int>& fieldTypes,
      const std::vector<std::string>& buttonLabels);

  virtual ~Dialog() {
    delete pAlphaNumericFieldType;
    delete pRegularExpressionFieldType;
  }

  virtual void run();

  int virtualize(int key);

  int driver(int command);

  int getKey();

  virtual NCursesFormField* getField(int index) const {
    return paFormFields[index];
  }
};

class DialogItem : public NCursesMenuItem {
  Dialog* pParentDialog;
  public:
  DialogItem(const char* s, Dialog* pDialog) : NCursesMenuItem(s) {
    pParentDialog = pDialog;
  }

  bool action() {
    const NCursesFormField* pUsernameFormField = pParentDialog->getField(0);
    const NCursesFormField* pPasswordFormField = pParentDialog->getField(1);
    char* username = pUsernameFormField->value();
    char* password = pPasswordFormField->value();
    int a = 0;
  }
};

// field item for static labels (adapted from ncurses demo.cc)
class LabelField : public NCursesFormField {
  public:
  LabelField(const std::string& title, int row, int column) :
    NCursesFormField(1, title.length(), row, column) {
      set_value(title.c_str());
      options_off(O_EDIT | O_ACTIVE);
    }
};

#endif	// DIALOG_H
