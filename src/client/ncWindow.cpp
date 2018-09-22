#include "ncWindow.h"
#include <cursesp.h>
#include <fstream>

void NCWindow::run() {
  NCursesPanel* pBackground = new NCursesPanel();
  /* DBG! */ std::ofstream dbgFile("dbg.out"); dbgFile << "NCWindow::run()" << std::endl;
  //TopMenu* pTopMenu = new TopMenu();
}
