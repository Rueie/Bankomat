#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    int port=8080;//порт сервера
    QApplication a(argc, argv);
    Mainwindow window(nullptr,port);//создали экземпляр клиента
    window.resize(250,250);//изменили размер клиента, дальше все масштабируется, так что на размер все равно
    window.show();
    return a.exec();
}
