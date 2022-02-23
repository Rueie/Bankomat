#include "mainwindow.h"

Mainwindow::Mainwindow(QWidget *parent,int new_port) : QWidget (parent)
{
    pages=new QStackedWidget;//создали полностью пустой список

    port=new_port;//загрузили порт в клиент

    QGroupBox *first=new QGroupBox;//контейнер страницы входа
    QGroupBox *second=new QGroupBox;//контейнер страницы клиента

    QVBoxLayout *first_page=new QVBoxLayout;//страница входа
    QVBoxLayout *second_page=new QVBoxLayout;//страница личного кабинета

    //создание полей для страниц
    info=new QLabel("Введите никнейм и пароль",this);//заголовок
    info->setAlignment(Qt::AlignCenter);//централизация содержимого в поле
    nik=new QLineEdit(this);//поле для ника
    nik->setAlignment(Qt::AlignCenter);//централизация содержимого в поле
    nik->setMaxLength(5);//поставили лимит символов на ввод
    password=new QLineEdit(this);
    password->setAlignment(Qt::AlignCenter);//централизация содержимого в поле
    password->setMaxLength(4);//поставили лимит символов на ввод
    password->setEchoMode(QLineEdit::Password);
    sign_in=new QPushButton("Войти",this);//заголовок - уведомление о блокировке
    message=new QLabel("",this);
    message->setAlignment(Qt::AlignCenter);//централизация содержимого в поле
    //вносим поля в список первой страницы
    first_page->addWidget(info);
    first_page->addWidget(nik);
    first_page->addWidget(password);
    first_page->addWidget(sign_in);
    first_page->addWidget(message);


    client=new QLabel("",this);//заголовок для данных о пользователе
    client->setAlignment(Qt::AlignHCenter);//централизация содержимого в поле
    money=new QLineEdit(this);
    money->setAlignment(Qt::AlignHCenter);//централизация содержимого в поле
    money->setMaxLength(6);//ограничили максимальное число символов
    money->setValidator(new QIntValidator(0,300000));//поставили ограничение на максимальную сумму и ввод только цифр
    take_money=new QPushButton("Take",this);//кнопка снять деньги
    put_money=new QPushButton("Put",this);//кнопка положить деньги
    message_for_client=new QLabel("",this);//сообщения для пользователя, что у него не хватает денег
    message_for_client->setAlignment(Qt::AlignHCenter);//централизация содержимого в поле
    on_main=new QPushButton("On main",this);//кнопка на главную
    //вносим поля в список второй страницы
    second_page->addWidget(client);
    second_page->addWidget(money);
    second_page->addWidget(take_money);
    second_page->addWidget(put_money);
    second_page->addWidget(message_for_client);
    second_page->addWidget(on_main);

    //вносим списки полей в соответствующие поля
    first->setLayout(first_page);
    second->setLayout(second_page);

    //вносим страницы в список страниц
    pages->addWidget(first);
    pages->addWidget(second);

    //создаем контейнер, где будут менятся страницы
    QVBoxLayout *box=new QVBoxLayout;
    //вносим список страниц в контейнер
    box->addWidget(pages);
    this->setLayout(box);
    pages->setCurrentIndex(0);//активируем первую страницу

    //подключения кнопок
    connect(sign_in,&QPushButton::clicked,this,&Mainwindow::SignIn);//авторизация
    connect(on_main,&QPushButton::clicked,this,&Mainwindow::On_main);//на главную
    connect(take_money,&QPushButton::clicked,this,&Mainwindow::Take_money);//снять деньги
    connect(put_money,&QPushButton::clicked,this,&Mainwindow::Put_money);//положить деньги
}

void Mainwindow::SignIn(){
    //очистка поля для ввода денег и поля сообщений в личном кабинете
    info->setText("");
    message->setText("");
    QString name=nik->text();//запомнили имя пользователя
    QString password_in;//пароль, который ввел пользователь в клиенте
    int sock;
    struct sockaddr_in address;//адресс сокета

    if((sock=socket(AF_INET,SOCK_STREAM,0))<0){//выделям сокет под общение с сервером
        qWarning("<Ошибка выделения сокета>");
        return;
    }

    //задаем параметры подключения
    address.sin_family=AF_INET;
    address.sin_port=htons(port);

    if(inet_pton(AF_INET,"127.0.0.1",&address.sin_addr)<=0){//настраиваем подключение по заданным параметрам
        qWarning("<Некорректный адресс>");
        return;
    }

    if(::connect(sock,(struct sockaddr*)&address,sizeof(address))<0){//подключаемся по настроенному каналу выше
        qWarning("<Ошибка подключения>");
        return;
    }

    send(sock,"s",1,0);//отправляем запрос типа 's' от слов 'sing in'

    char *buffer_out;//буфер для отправки сообщений серверу
    char buffer_in[1024]={0};//буфер для получения данных от сервера
    read(sock,buffer_in,1);//чтение ответа сервера
    if(buffer_in[0]!='+'){//если запрос был неудачен, то выходим из общения с сервером
        qWarning("Неудачный запрос на сервер");
        return;
    }
    //преобразуем имя пользователя в строку для отправки его на сервер
    QByteArray str=name.toLocal8Bit();
    buffer_out=str.data();
    send(sock,buffer_out,sizeof(buffer_out),0);//отправка имени пользователя на сервер
    memset(buffer_in,0,sizeof(buffer_in));
    read(sock,buffer_in,1);//чтение ответа сервера
    if(buffer_in[0]!='+'){//если пользователь не найден, то пишем в клиент об этом и прекращаем сеанс связи
        info->setText("Такого клиента не сущетсвует");
        return;
    }
    memset(buffer_in,0,1);//очистка буфера
    read(sock,buffer_in,1);//чтение статуса пользователя
    if(buffer_in[0]!='+'){//если пользователь заблокирован, то выводим сообщение в клиенте и прекращаем сеанс связи
        info->setText("Данный клиент заблокирован\nОбратитесь к администратору");
        return;
    }
    memset(buffer_in,0,1);//очистка буфера
    read(sock,buffer_in,1);//чтение оставшегося числа попыток входа
    if(buffer_in[0]!='+'){//если попытка была последней и она была потрачена, то вывод об этом сообщений и прекращение сеанся связи
        info->setText("Данный клиент заблокирован\nОбратитесь к администратору");
        message->setText("У вас больше не осталось попыток\nАккаунт блокируется\n");
        message->show();
        return;
    }
    //считываем пароль из поля, введеный пользователем, и преобразуем в массив из char
    password_in=password->text();
    str=password_in.toLocal8Bit();
    buffer_out=str.data();
    //--------------------конец преобразования
    send(sock,buffer_out,sizeof(buffer_out),0);//отправка пароля на сервер
    memset(buffer_in,0,1);//очистка буфера
    read(sock,buffer_in,1);//чтение ответа сервера
    if(buffer_in[0]!='+'){//если пароль был ошибочным, то выводим сообщение об этом и прекращаем сеанс связи
        info->setText("Неверный пароль\nПопробуйте еще раз");
        return;
    }
    memset(buffer_in,0,1);//очистка буфера
    read(sock,buffer_in,1024);//чтение счета пользователя
    //преобразование массива char в Qstring
    QString *money_string=new QString(buffer_in);
    money_string->truncate(sizeof(buffer_in));
    //-------------конец преобразования
    coins=money_string->toInt();//запоминаем счет пользователя
    //вносим данные для заголовка на странице личного кабинета и чистим все поля на второй странице
    client->setText("Здравствуйте, "+name+"\nВаш баланс: "+money_string+"\nВведите сумму для дальнейших операций с ней:");
    message_for_client->setText("");
    money->setText("");
    pages->setCurrentIndex(1);//активируем вторую страницу
    return;
}

void Mainwindow::On_main(){
    //очистка и редактирование полей на первой странице
    info->setText("Введите никнейм и пароль");
    message->setText("");
    nik->setText("");
    password->setText("");
    pages->setCurrentIndex(0);//активирование первой страницы
    return;
}

void Mainwindow::Put_money(){
    message_for_client->setText("");//очистка поля сообщения в кабинете клиента
    coins+=money->text().toInt();//подсчет нового счета клиента
    Send_money_to_server();//соединяемся с сервером и отправляем ему измененный счет
    return;
}

void Mainwindow::Take_money(){
    if(coins<money->text().toInt()){//если пытаемся снять больше, чем на счету, то вывод сообщения в личном кабинете
        message_for_client->setText("Не возможно провести эту операцию\nНедостаточно средств на счету");
        return;
    }
    coins-=money->text().toInt();//подсчет нового счета клиента
    Send_money_to_server();//соединяемся с сервером и отправляем ему измененный счет
    return;
}

void Mainwindow::Send_money_to_server(){
    int sock;
    struct sockaddr_in address;//адресс сокета

    char buffer_in[1024]={0};//буфер для общения с сервером

    if((sock=socket(AF_INET,SOCK_STREAM,0))<0){//устанавливаем сокет, если провал - вывод сообщения и прекращение сеанса связи с сервером
        qWarning("<Ошибка выделения сокета>");
        return;
    }

    //установка параметров соединения
    address.sin_family=AF_INET;
    address.sin_port=htons(port);

    if(inet_pton(AF_INET,"127.0.0.1",&address.sin_addr)<=0){//установка канала связи, если провал, то выводим сообщение и прекращаем
        //сеанс связи с сервером
        qWarning("<Некорректный адресс>");
        return;
    }

    if(::connect(sock,(struct sockaddr*)&address,sizeof(address))<0){//установка связи по каналу связи, если провал, то выводим
        //сообщение и прекращаем сенас связи с сервером
        qWarning("<Ошибка подключения>");
        return;
    }

    send(sock,"m",1,0);//отправляем запрос на сервер типа 'm' от слова 'money'
    read(sock,buffer_in,1);//слушаем ответ сервера
    if(buffer_in[0]!='+'){//если неудача, то пишем сообщение и прекращаем сеанс связи с сервером
        qWarning("Неудачный запрос на сервер");
        return;
    }
    //преобразуем Qstring в массив из char
    char *buffer_out;
    QByteArray str=nik->text().toLocal8Bit();
    buffer_out=str.data();
    //-------------------конец преобразования
    send(sock,buffer_out,sizeof(buffer_out),0);//отправили ник пользователя
    memset(buffer_in,0,1);//чистим буфер
    read(sock,buffer_in,1);//слушаем ответ от сервера
    if(buffer_in[0]=='-'){//если такого клиента нет, то пишем сообщение и прекращаем сеанс связи с сервером
        qWarning("Ошибка поиска клиента на сервере");
        return;
    }
    //преобразование счета (число) в массив из char
    str=QString::number(coins).toLocal8Bit();
    buffer_out=str.data();
    //----------------конец преобразования
    send(sock,buffer_out,sizeof(buffer_out),0);//отправили на сервер новый счет пользователя
    memset(buffer_in,0,1);//чистим буфер
    read(sock,buffer_in,1);//слушаем сервер
    if(buffer_in[0]=='-'){//если операция не была выполнена успешно, то выводим сообщение и прекращаем сеанс связи
        qWarning("Ошибка выполнения операции на сервере");
        return;
    }
    On_main();//переходим на главный экран
    return;
}
