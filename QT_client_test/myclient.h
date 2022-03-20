#ifndef _MyClient_h_
#define _MyClient_h_

#include <QWidget>
#include <QTcpSocket>
#include <QTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTime>
#include <QStackedWidget>
#include <QGroupBox>
#include <QIntValidator>

class QTextEdit;
class QLineEdit;

class MyClient : public QWidget {
Q_OBJECT
private:
    QTcpSocket* m_pTcpSocket;//сокет сервера
    QLineEdit*  m_ptxtInput;//данные, которые пришли с сервера
    quint16     m_nNextBlockSize;//размер блока данных

    QStackedWidget *pages;//2 страницы
    //первая страница
    QLabel *info;//заголовок (приветствие или уведомление о блокировке)
    QLineEdit *nik;//поле для ввода ника пользователя
    QLineEdit *password;//поле для ввода пароля пользователя
    QPushButton *sign_in;//кнопка авторизации
    QLabel *message;//сообщение (уведомление о блокировке из-за множества неудачных попыток входа)
    //вторая страница
    QLabel *client;//информация о пользователе: его имя и счет
    QLineEdit *money;//поле для ввода суммы денег для дальнейших операций
    QPushButton *take_money;//кнопка взять деньги со счета
    QPushButton *put_money;//кнопка положить деньги на счет
    QPushButton *on_main;//кнопка на главную
    QLabel * message_for_client;//поле сообщения для клиента (случай, когда на счету у пользователя меньше денег, чем он пытается снять)
    int port;//порт
    int coins;//сумма на счету у пользователя при авторизации, затем поситанная сумма в процессе операций, которая потом будет отправлена на сервер

    int number_of_mes;//идентификатор сообщения, нужен для опознавания того, где обрабатывать сообщение
    QString mes;//само сообщение

    bool ErrorFlag=false;//флаг ошибок, чтобы сообщения об ошибке не затирались пре переключении страниц

public:
    MyClient(const QString& strHost, int nPort, QWidget* pwgt = 0);//конструктор сервера
    void SignIn(int flag=0);//авторизация
    void On_main();//переход на главный экран из личного кабинета пользователя
    void Put_money();//положить деньги
    void Take_money();//взять деньги со счета
    void Send_money_to_server();//отправка <нового> счета пользователя на сервер

private slots:
    void slotReadyRead   (                            );//в случае полечения сообщения от сервера - обрабатываем его
    void slotError       (QAbstractSocket::SocketError);//в случае ошибки клиент выводит ошибки на эркан приложения
    void slotSendToServer();//для отправки сообщения на сервер
};
#endif  //_MyClient_h_
