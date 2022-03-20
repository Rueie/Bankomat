#ifndef MYSERVER_H
#define MYSERVER_H

#include <QApplication>
#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QLabel>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>

class QTcpServer;
class QTextEdit;//поле, где происходит эхо-печать событий на сервере
class QTcpSocket;

class MyServer:public QWidget
{
    Q_OBJECT
private:
    QString file_address="/home/rueie/Qt_projects/1.1.server/server_qt/users.json";//адрес файла json с данными
    QTcpServer* m_ptcpServer;//сам сервер
    QTextEdit* m_ptxt;//само сообщение
    quint16 m_nNextBlockSize;//размер сообщения (в байтах)
    QTcpSocket* pClientSocket;//сокет, для общения с клиентом
    //данные сообщения, а именно номер сообщения и его содержимое
    QString mes;
    int number_of_mes;
    //данные о пользователе
    QString name;//имя
    QString password;//пароль
    QString status;//статус
    int number_of_attempts;//число сотавшихся попыток
    int money;//счет
private:
    void sendToClient(QTcpSocket* pSocket);//отправка на клиент сообщения
    void CheckUser(int flag);//проверка данных, связанных с пользователем
    void ChangeMoney();//изменение счета в файле .json
public:
    MyServer(int nPort,QWidget* pwgt=0);
public slots:
    virtual void slotNewConnection();//в случае нового подключения нужен порт, его тут и выделяем
            void slotReadClient();//чтение сообщения от клиента
};

#endif // MYSERVER_H
