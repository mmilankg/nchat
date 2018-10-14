#include "acceptor.h"

void Acceptor::acceptConnection()
{
    Socket * pClientSocket = new Socket(pListeningSocket->acceptConnection());
    sendSocket(pClientSocket);
}
