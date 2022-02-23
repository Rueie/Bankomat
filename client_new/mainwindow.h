#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QIntValidator>
#include <QGroupBox>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

class Mainwindow : public QWidget
{
    Q_OBJECT
public:
    explicit Mainwindow(QWidget *parent = nullptr,int new_port=0);
    void SignIn();//авторизация
    void On_main();//переход на главный экран из личного кабинета пользователя
    void Put_money();//положить деньги
    void Take_money();//взять деньги со счета
    void Send_money_to_server();//отправка <нового> счета пользователя на сервер
protected:
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

};

#endif // MAINWINDOW_H
