#include <QApplication>
#include "myclient.h"

int main(int argc, char** argv)
{
    int port=8080;
    QApplication app(argc, argv);
    MyClient     client("localhost", port);

    client.show();

    return app.exec();
}
