#include "loginDialog.h"
#include "socket.h"
#include "user.h"
#include <cursesapp.h>
#include <cursesm.h>
#include <cursesf.h>

/* DBG: may be needed in a later implementation. */
int verbosityLevel = 0;

class NCClient : public NCursesApplication {
    Socket socket;
    /* DBG: The user object is not currently used, but it seems natural that the application itself should have the
     * information on which user is logged on through it. */
    User user;

protected:
    int  titlesize() const { return 1; }
    void title();

public:
    /* Hard-code server address (127.0.0.1, for testing) and port number (10001). */
    NCClient() : NCursesApplication(true), socket("127.0.0.1", 10001, 1024) {}
    int      run();
    Socket & getSocket() { return socket; }
};

void NCClient::title()
{
    const char * const titleText = "ncurses-based communication program";
    const int          len       = ::strlen(titleText);

    titleWindow->bkgd(screen_titles());
    titleWindow->addstr(0, (titleWindow->cols() - len) / 2, titleText);
    titleWindow->noutrefresh();
}

int NCClient::run()
{
    LoginDialog loginDialog(&socket, 9, 35, 0, 0);
    loginDialog.run();
    return 0;
}

static NCClient * pWindow = new NCClient();
