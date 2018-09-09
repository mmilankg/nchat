#include "socket.h"
#include "loginDialog.h"
#include <cursesapp.h>
#include <cursesm.h>
#include <cursesf.h>

class NCClient : public NCursesApplication {
  Socket socket;
protected:
  int titlesize() const { return 1; }
  void title();
public:
  /* Hard-code server address (192.168.0.10) and port number (10001). */
  //NCClient() : NCursesApplication(true), socket("192.168.0.10", 10001, 1024) { }
  /* Hard-code server address (127.0.0.1) and port number (10001). */
  NCClient() : NCursesApplication(true), socket("127.0.0.1", 10001, 1024) { }
  int run();
  Socket& getSocket() { return socket; }
};

void NCClient::title() {
  const char* const titleText = "ncurses-based communication program";
  const int len = ::strlen(titleText);

  titleWindow->bkgd(screen_titles());
  titleWindow->addstr(0, (titleWindow->cols() - len) / 2, titleText);
  titleWindow->noutrefresh();
}

int NCClient::run() {
  LoginDialog loginDialog(&socket, 9, 35, 0, 0);
  loginDialog.run();
  return 0;
}

static NCClient* pWindow = new NCClient();
