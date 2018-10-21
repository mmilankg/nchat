#include "acceptor.h"

void Acceptor::acceptConnection()
{
    Socket * pClientSocket = new Socket(listeningSocket.acceptConnection());
    sendSocket(pClientSocket);
}
