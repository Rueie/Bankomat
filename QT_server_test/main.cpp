#include <QApplication>
#include <QtGui>
#include "myserver.h"

int main(int argc, char *argv[])
{
    int port=8080;
    QApplication a(argc, argv);
    MyServer server(port);

    server.show();

    return a.exec();
}
