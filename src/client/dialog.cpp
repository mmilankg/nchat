#include "dialog.h"

Dialog::Dialog(int height, int width, int y0, int x0) :
  dialogHeight(height), dialogWidth(width),
  dialogY(y0), dialogX(x0),
  inForm(true), inMenu(false),
  NCursesForm(height, width, (NCursesForm::lines() - height) / 2, (NCursesForm::cols() - width) / 2),
  NCursesMenu(1, width, (NCursesForm::lines() + height) / 2, (NCursesForm::cols() - width) / 2),
  paFormFields(NULL), paMenuItems(NULL),
  pAlphaNumericFieldType(NULL),
  pRegularExpressionFieldType(NULL)
{ }

/* DBG: perhaps there's a better place for this. */
static const int CMD_QUIT   = MAX_COMMAND + 1;
static const int CMD_ACTION = MAX_COMMAND + 2;

void Dialog::run() {
  NCursesMenu::post();
  NCursesMenu::show();
  /*
   * DBG: Selects the last (invisible) item to
   * leave the appearance that no items are selected.
   */
  NCursesMenu::driver(REQ_LAST_ITEM);
  NCursesMenu::refresh();
  NCursesForm::post();
  NCursesForm::show();
  NCursesForm::refresh();

  int command, error, key;
  while ((command = virtualize(key = getKey())) != CMD_QUIT) {
    int fieldIndex = ::field_index(::current_field(form));
    int itemIndex = ::item_index(::current_item(menu));
    error = driver(command);
    /* DBG! */
    if (inMenu) {
      if (command == DIALOG_ENTER) {
	bool quit = paMenuItems[itemIndex]->action();
	if (quit)
	  break;
      }
    }
  }

  NCursesForm::unpost();
  NCursesForm::hide();
  NCursesForm::refresh();
  NCursesMenu::unpost();
  NCursesMenu::hide();
  NCursesMenu::refresh();
}

int Dialog::virtualize(int key) {
  switch(key) {
    case KEY_HTAB	: return(DIALOG_NEXT);
    case KEY_BTAB	: return(DIALOG_PREV);
    case KEY_ENTER	:
    case CTRL('M')	:
    case CTRL('J')	: return(DIALOG_ENTER);
    case KEY_BACKSPACE	: return(REQ_DEL_PREV);
    case CTRL('X')	: return(CMD_QUIT);
    default		: return(key);
  }
}

int Dialog::driver(int command) {
  /*
   * The first if-block deals with switching between the
   * form and the menu.  It detects when the next entity
   * is chosen from the last entity of a group, or when
   * the previous entity is chosen from the first entity
   * of a group.
   */
  if (inForm) {
    int fieldIndex = ::field_index(::current_field(form));
    int nFields = NCursesForm::count();
    if ((fieldIndex == (nFields - 1)) && (command == DIALOG_NEXT)) {
      inForm = false;
      inMenu = true;
      /*DBG!
       * Some command needs to be sent to the driver for
       * the field to pick up the entered data.  Sending
       * REQ_END_FIELD seems the least disruptive to do
       * the job.  This all * seems very inelegant, and
       * will have to be reworked.  Ideally, both fields
       * should be deactivated once the cursor moves to
       * the menu.
       */
      NCursesForm::driver(REQ_END_FIELD);
      NCursesMenu::set_current(*paMenuItems[0]);
      NCursesMenu::position_cursor();
      NCursesForm::refresh();
      NCursesMenu::refresh();
      return 0;
    }
    /*
     * Assume that the first field is always a label and the
     * first active field starts with index 1 (second field).
     */
    if ((fieldIndex == 1) && (command == DIALOG_PREV)) {
      inForm = false;
      inMenu = true;
      /*
       * DBG: Select the second to last menu item (last is
       * the fake item used only to leave an impressions that
       * no items are selected).
       */
      NCursesMenu::set_current(*paMenuItems[NCursesMenu::count() - 2]);
      NCursesMenu::position_cursor();
      NCursesForm::refresh();
      NCursesMenu::refresh();
      return 0;
    }
  }
  else if (inMenu) {
    int itemIndex = ::item_index(::current_item(menu));
    int nItems = NCursesMenu::count();
    if ((itemIndex == (nItems - 2)) && (command == DIALOG_NEXT)) {
      inForm = true;
      inMenu = false;
      /*
       * Go to the second form field as the first one is a label.
       */
      NCursesForm::set_current(*paFormFields[1]);
      NCursesForm::position_cursor();
      NCursesForm::refresh();
      /*
       * DBG: Select the last (fake) manu item to leave an appearance
       * that none is selected.
       */
      NCursesMenu::set_current(*paMenuItems[NCursesMenu::count() - 1]);
      NCursesMenu::refresh();
      return 0;
    }
    if ((itemIndex == 0) && (command == DIALOG_PREV)) {
      inForm = true;
      inMenu = false;
      NCursesForm::set_current(*paFormFields[NCursesForm::count() - 1]);
      NCursesForm::position_cursor();
      NCursesForm::refresh();
      /*
       * DBG: Select the last (fake) manu item to leave an appearance
       * that none is selected.
       */
      NCursesMenu::set_current(*paMenuItems[NCursesMenu::count() - 1]);
      NCursesMenu::refresh();
      return 0;
    }
  }

  /*
   * The second if-block just converts the command to
   * either a form- or a menu-command, and passes it
   * to the driver of the corresponding inherited class.
   */
  int result;
  if (inForm) {
    if (command == DIALOG_NEXT)
      command = REQ_NEXT_FIELD;
    else if (command == DIALOG_PREV)
      command = REQ_PREV_FIELD;
    else if (command == DIALOG_ENTER)
      command = REQ_NEW_LINE;
    result = NCursesForm::driver(command);
  }
  else if (inMenu) {
    if (command == DIALOG_NEXT)
      command = REQ_NEXT_ITEM;
    else if (command == DIALOG_PREV)
      command = REQ_PREV_ITEM;
    else if (command == DIALOG_ENTER)
      command = REQ_NEW_LINE;
    result = NCursesMenu::driver(command);
  }
  return result;
}

int Dialog::getKey() {
  if (inForm)
    return NCursesForm::getKey();
  else
    return NCursesMenu::getKey();
}
