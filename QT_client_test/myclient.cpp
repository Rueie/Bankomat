#include "myclient.h"

MyClient::MyClient(const QString& strHost,int nPort,QWidget* pwgt):QWidget(pwgt), m_nNextBlockSize(0){
    m_pTcpSocket = new QTcpSocket(this);//Создали сокет для подключений
    m_pTcpSocket->connectToHost(strHost, nPort);//подключились к созданному сокету по порту, который был задан в конструктор

    connect(m_pTcpSocket, SIGNAL(readyRead()), SLOT(slotReadyRead()));//подключаем сигнал чтения к слоту сервера
    connect(m_pTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),this, SLOT(slotError(QAbstractSocket::SocketError)));//подключаем сигнал при возникновении ошибок с подключением к серверу

    m_ptxtInput = new QLineEdit;
    connect(m_ptxtInput, SIGNAL(returnPressed()),this,SLOT(slotSendToServer()));//подключили сигнал для отправки сообщения для слота сервера

    pages=new QStackedWidget;//создали полностью пустой список

    port=nPort;//загрузили порт в клиент

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
    connect(sign_in,&QPushButton::clicked,this,&MyClient::SignIn);//авторизация
    connect(on_main,&QPushButton::clicked,this,&MyClient::On_main);//на главную
    connect(take_money,&QPushButton::clicked,this,&MyClient::Take_money);//снять деньги
    connect(put_money,&QPushButton::clicked,this,&MyClient::Put_money);//положить деньги

    setWindowTitle("Rueie's bank");
}

void MyClient::slotReadyRead()
{
    QDataStream in(m_pTcpSocket);
    in.setVersion(QDataStream::Qt_4_2);
    for (;;) {
        if (!m_nNextBlockSize) {
            if (m_pTcpSocket->bytesAvailable() < sizeof(quint16)) {
                break;
            }
            in >> m_nNextBlockSize;
        }

        if (m_pTcpSocket->bytesAvailable() < m_nNextBlockSize) {
            break;
        }
        in >> number_of_mes;
        in>>mes;
        in>>mes;
        switch (number_of_mes) {
        case 0:{
            SignIn(1);
            break;
        }
        case 1:{
            SignIn(2);
            break;
        }
        case 2:{
            SignIn(3);
            break;
        }
        case 3:{
            SignIn(4);
            break;
        }
        }
        m_nNextBlockSize = 0;
    }
}

void MyClient::slotError(QAbstractSocket::SocketError err)
{
    QString strError =
        "Ошибка: " + (err == QAbstractSocket::HostNotFoundError ?
                     "Не был найдет серер" :
                     err == QAbstractSocket::RemoteHostClosedError ?
                     "Сервер закрыт" :
                     err == QAbstractSocket::ConnectionRefusedError ?
                     "В соединении было отказано" :
                     QString(m_pTcpSocket->errorString())
                    );
    //очистка полей с сообщениями для клиентов на обеих страницах, для того, чтобы вывести данные о ошибке, связанной с сервером
    message->clear();
    message->setText(strError);
    message_for_client->clear();
    message_for_client->setText(strError);
    ErrorFlag=true;//установили влаг, чтобы сообщения не затирались пре переключениях старниц
}

void MyClient::slotSendToServer()
{
    QByteArray  arrMes;
    QDataStream out(&arrMes, QIODevice::WriteOnly);//пишем данные теперь в arrMes
    out.setVersion(QDataStream::Qt_4_2);
    out << quint16(0) << number_of_mes<<"/"<<mes;//записали данные в формате <размер сообщения, пока равный 0><номер сообщения></><само сообщение>

    out.device()->seek(0);//перешли на начало всего сообщения
    out << quint16(arrMes.size() - sizeof(quint16));//считали длину всего сообщения и записали его в начало

    m_pTcpSocket->write(arrMes);//отправили сообщение на сервер
}

void MyClient::SignIn(int flag)
{
    switch (flag) {
    case 0:{//отправляем ник пользователя
        number_of_mes=0;
        mes=nik->text();
        slotSendToServer();
        if(ErrorFlag==false)//если у нас есть ошибка, связаннас с сервером, то не очищаем поле ошибок
            message->clear();
        break;
    }
    case 1:{//проверка на существование пользователя
        if(mes.compare("exists")!=0)
            message->setText("Данного пользователя не существует");
        break;
    }
    case 2:{//проверка на блокировку пользователя
        if(mes.compare("blocked")==0)
            message->setText("Данный клиент заблокирован, обратитесь к администратору");
        else{//и отправка пароля в случае, если он не заблокирован
            //считали пароль с клиента и отправили на сервер
            mes.clear();
            mes.append(password->text());
            number_of_mes=1;
            slotSendToServer();
        }
        break;
    }
    case 3:{//проверка на корректность пароля
        if(mes.compare("correct")==0){//переход на страницу пользователя
            pages->setCurrentIndex(1);//переключаемся на вторую страницу
        }
        else{//вывод сообщения о неверности пароля
            message->setText("Неверный пароль");
        }
        break;
    }
    case 4:{//инициализация страницы пользователя, имени, счета, считывание в клиент счета пользователя
        QString for_client;
        for_client.append("Добрый день, ");
        for_client.append(nik->text());
        for_client.append("\nВаш счет:");
        for_client.append(mes);
        coins=mes.toInt();
        client->setText(for_client);
        if(ErrorFlag==false)
            message_for_client->clear();
        break;
    }
    }//switch
}

void MyClient::On_main(){
    //очистка и редактирование полей на первой странице
    info->setText("Введите никнейм и пароль");
    if(ErrorFlag==false)
        message->setText("");
    nik->setText("");
    password->setText("");
    pages->setCurrentIndex(0);//активирование первой страницы
}

void MyClient::Put_money(){
    message_for_client->setText("");//очистка поля сообщения в кабинете клиента
    coins+=money->text().toInt();//подсчет нового счета клиента
    Send_money_to_server();//соединяемся с сервером и отправляем ему измененный счет
    return;
}

void MyClient::Take_money(){
    if(coins<money->text().toInt()){//если пытаемся снять больше, чем на счету, то вывод сообщения в личном кабинете
        message_for_client->setText("Не возможно провести эту операцию\nНедостаточно средств на счету");
        return;
    }
    coins-=money->text().toInt();//подсчет нового счета клиента
    Send_money_to_server();//соединяемся с сервером и отправляем ему измененный счет
    return;
}

void MyClient::Send_money_to_server(){
    //создаем сообщение, в котором находится новый счет пользователя
    number_of_mes=2;
    mes.clear();
    mes.append(QString::number(coins));
    slotSendToServer();
    On_main();
}
