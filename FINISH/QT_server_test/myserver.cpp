#include "myserver.h"
#include <QtGui>
#include <QMessageBox>

MyServer::MyServer(int nPort, QWidget* pwgt):QWidget(pwgt),m_nNextBlockSize(0)
{
    m_ptcpServer=new QTcpServer(this);//создали сервер
    m_ptxt=new QTextEdit;//поле, где будет производится эхо-печать
    if(!m_ptcpServer->listen(QHostAddress::Any,nPort)){
      m_ptxt->append("Не удалось создать сервер на выделенном порте\nНомер порта:");
      m_ptxt->append(QString::number(nPort));
      m_ptcpServer->close();//закрыли сервер
      return;
    }
    connect(m_ptcpServer,SIGNAL(newConnection()),this,SLOT(slotNewConnection()));//связали сигнал нового подключения с функцией создания нового порта
    m_ptxt->setReadOnly(true);

    //создаем бокс, в котором будет хранится m_ptxt, и помещаем его в заголов окна сервера
    QVBoxLayout* box=new QVBoxLayout;
    box->addWidget(new QLabel("Server"));
    box->addWidget(m_ptxt);
    setLayout(box);
}

void MyServer::slotNewConnection(){
        pClientSocket = m_ptcpServer->nextPendingConnection();//создали порт для работы с клиентом и связали с обработчиками чтения и отключения
        connect(pClientSocket, SIGNAL(disconnected()),pClientSocket, SLOT(deleteLater()));
        connect(pClientSocket, SIGNAL(readyRead()), this,SLOT(slotReadClient()));
}

void MyServer::slotReadClient()
{
    QTcpSocket* pClientSocket = (QTcpSocket*)sender();//создали порт-вещатель
    QDataStream in(pClientSocket);//сделали поток данных из порта-вещателя
    in.setVersion(QDataStream::Qt_4_2);
    for (;;) {
        if (!m_nNextBlockSize) {//пока есть сообщения
            if (pClientSocket->bytesAvailable() < sizeof(quint16)) {
                break;
            }
            in >> m_nNextBlockSize;//чтение размера сообщения
        }

        if (pClientSocket->bytesAvailable() < m_nNextBlockSize) {//если длина сообщения не равна действительному, то выходим из чтения данных
            break;
        }
        in>>number_of_mes>>mes>>mes;//чтение номера сообщения и его содержимого
        //m_ptxt->append(QString::number(number_of_mes)+"<"+mes+">");//эхо-печать в окно сервера (можно включать для проверки наботы)
        switch (number_of_mes){//в зависимости от номера сообщения м.б. разная его обработка
        case 0:{
            CheckUser(0);
            break;
        }
        case 1:{
            CheckUser(1);
            break;
        }
        case 2:{
            ChangeMoney();
            break;
        }
        }
        m_nNextBlockSize = 0;//обнуляем длину, иначе будет ересь при повторной связи с сервером
    }
}

void MyServer::sendToClient(QTcpSocket* pSocket)
{
    //полный аналог с отправкой сообщений от клиента
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_2);
    out << quint16(0) << number_of_mes<<"/"<<mes;;

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    pSocket->write(arrBlock);
}

void MyServer::CheckUser(int flag){
    QFile json_file(file_address);//файл json с данными
    if(!json_file.exists()){//проверям доступность файла json
        m_ptxt->append("Файл json не доступен");
        exit (1);
    }
    else if(!json_file.open(QIODevice::ReadOnly)){//открываем файл json только для чтения и проверяем, выполнилось ли открытие
        m_ptxt->append("Ошибка открытия файла json");
        exit (2);
    }
    else{
        QString reader;//строка для чтения в неё файла json
        QJsonDocument json_doc;//для работы с файлом json
        QJsonArray json_arr;//для работы с массивами в json
        int number=0;//номер объекта в массиве json
        QJsonObject json_obj;//для работы с объектами json
        int flag_nik=0;//флаг того, что подключающийся человек был найден среди клиентов в json
        QString username;//ник клиента

        reader=json_file.readAll();//читаем весь файл с данными о клиентах
        json_doc=QJsonDocument::fromJson(reader.toUtf8());//формируем объект типо jsondocument
        json_arr=json_doc.array();//получаем из json_doc массив с клиентами

        switch (flag){
        case 0:{
            //чистка данных на сервере о подключенном ранее клиенте
            name.clear();
            //запомнили имя нового подключенного клиента
            name.append(mes);
            mes.clear();
            while(1){//ищем клиента
                json_obj=json_arr.at(number).toObject();//выделяем очередной объект среди массива клиентов
                if(json_obj.isEmpty()){//не нашли его, т.е. дошли до конца файла
                    m_ptxt->append("Пользователя <"+name+"> нет в базе данных");
                    m_ptxt->append("Отправление сообщения на клиент");//оно стоит в else к флагу
                    flag_nik=1;//активаируем флаг, 1=нет такого клиента, 0=есть
                    mes.append("Not exists");
                    sendToClient(pClientSocket);
                    break;
                }
                username=json_obj["nik"].toString();//выделяем имя из объекта (пользователя)
                if(username.compare(name)==0){//нашли клиента
                    flag_nik=0;//деактивируем флаг
                    break;
                }
                number++;//идем к следующему объекту (пользователю)
            }
            if(flag_nik==0){//если нашли пользователя, то есть смысл дальше общаться с клиентом
                m_ptxt->append("Пользователь <"+name+"> есть в базе данных\nПроверка статуса пользователя");
                mes.append("exists");
                sendToClient(pClientSocket);//отправили на клиент сообщение о том, что такой пользователь существует

                number_of_mes=1;
                mes.clear();

                number_of_attempts=json_obj["attempts"].toInt();//считываем число попыток уже у найденного клиента
                if(number_of_attempts==0){//если число попыток для входа =0, то нет смысла дальше работать, меняем на blocked в .json и потом при проверке выйдем из входа
                    json_file.close();//закрыли файл json
                    //дальше идет редактирование числа попыток клиента в json, т.к. при вводе верного пароля они должны
                    //сброситься в начальное состояние, т.е. в 3
                    QFile json_file_edit(file_address);//снова задали файл json
                    if(!json_file_edit.open(QIODevice::WriteOnly|QIODevice::Truncate)){//открыли этот файл только на запись
                        //затерев все содержимое
                        qWarning("Файл json не открылся");
                        exit (3);
                    }
                    json_arr.removeAt(number);//удаляем объект, который надо редактировать в JsonDocument
                    json_obj.remove("status");//удаляем поле, которое надо отредактировать в клиенте
                    json_obj.insert("status","blocked");//вставляем поле со значением заблокирован
                    json_obj.remove("attempts");//удаляем поле, которое надо отредактировать в клиенте
                    json_obj.insert("attempts",3);//снова вставляем поле с начальным значение '3'
                    json_arr.insert(number,json_obj);//вставляем отредактированный объект в массив json
                    QJsonDocument new_doc;//создаем новый JsonDocument, т.к. в старом хранятся оставшиеся
                    //неотредактированные записи о пользователях
                    new_doc.setArray(json_arr);//вносим отредактированный массив json в JsonDocument
                    json_file_edit.write(new_doc.toJson());//записываем в файл json данные о пользователях
                    json_file_edit.close();//закрыли файл json
                }

                status=json_obj["status"].toString();//считываем статус уже у найденного клиента
                if(status.compare("blocked")==0){//если статус 'заблокирован'(blocked), то сообщаем на клиент
                    mes.append("blocked");
                    m_ptxt->append("Пользователь <"+name+"> заблокирован.\nОтправка сообщения на клиент");
                }
                else{
                    mes.append("normal");
                    m_ptxt->append("Пользователь <"+name+"> не заблокирован.\nОжидание пароля");
                    password=json_obj["password"].toString();//считываем статус уже у найденного клиента
                    money=json_obj["money"].toInt();
                }
                sendToClient(pClientSocket);//отправка сообщения для клиента со статусом пользователя
            }
            break;
        }
        case 1:{
            while(1){//ищем клиента
                json_obj=json_arr.at(number).toObject();//выделяем очередной объект среди массива клиентов
                username=json_obj["nik"].toString();//выделяем имя из объекта (пользователя)
                if(username.compare(name)==0){//нашли клиента
                    flag_nik=0;//деактивируем флаг
                    break;
                }
                number++;//идем к следующему объекту (пользователю)
            }
            if(flag_nik==0){
                json_file.close();//закрыли файл json
                //дальше идет редактирование числа попыток клиента в json, т.к. при вводе верного пароля они должны
                //сброситься в начальное состояние, т.е. в 3
                QFile json_file_edit(file_address);//снова задали файл json
                if(!json_file_edit.open(QIODevice::WriteOnly|QIODevice::Truncate)){//открыли этот файл только на запись
                    //затерев все содержимое
                    qWarning("Файл json не открылся");
                    exit (3);
                }
                json_arr.removeAt(number);//удаляем объект, который надо редактировать в JsonDocument
                json_obj.remove("attempts");//удаляем поле, которое надо отредактировать в клиенте

                number_of_mes=2;
                if(password.compare(mes)==0){
                    mes.clear();
                    mes.append("correct");
                    m_ptxt->append("Верный пароль\nПользователь <"+name+"> входит в личный кабинет");
                    json_obj.insert("attempts",3);//снова вставляем поле с начальным значение '3'
                }
                else {
                    mes.clear();
                    mes.append("uncorrect");
                    m_ptxt->append("Неверный пароль\nОтправка сообщения на клиент");
                    json_obj.insert("attempts",number_of_attempts-1);//уменьшаем число попыток входа на 1
                    number_of_attempts--;;
                }
                json_file.close();//закрыли файл json

                //дальше идет редактирование числа попыток клиента в json, т.к. при вводе верного пароля они должны
                json_arr.insert(number,json_obj);//вставляем отредактированный объект в массив json
                QJsonDocument new_doc;//создаем новый JsonDocument, т.к. в старом хранятся оставшиеся
                //неотредактированные записи о пользователях
                new_doc.setArray(json_arr);//вносим отредактированный массив json в JsonDocument
                json_file_edit.write(new_doc.toJson());//записываем в файл json данные о пользователях
                json_file_edit.close();//закрыли файл json
                sendToClient(pClientSocket);
                mes.clear();
                number_of_mes=3;
                mes.append(QString::number(money));
                sendToClient(pClientSocket);
                break;
            }
        }
        }
    }
}

void MyServer::ChangeMoney(){
    QFile json_file(file_address);//файл json с данными
    if(!json_file.exists()){//проверям доступность файла json
        m_ptxt->append("Файл json не доступен");
        exit (1);
    }
    else if(!json_file.open(QIODevice::ReadOnly)){//открываем файл json только для чтения и проверяем, выполнилось ли открытие
        m_ptxt->append("Ошибка открытия файла json");
        exit (2);
    }
    QString reader;//строка для чтения в неё файла json
    QJsonDocument json_doc;//для работы с файлом json
    QJsonArray json_arr;//для работы с массивами в json
    int number=0;//номер объекта в массиве json
    QJsonObject json_obj;//для работы с объектами json
    int flag_nik=0;//флаг того, что подключающийся человек был найден среди клиентов в json
    QString username;//ник клиента

    reader=json_file.readAll();//читаем весь файл с данными о клиентах
    json_doc=QJsonDocument::fromJson(reader.toUtf8());//формируем объект типо jsondocument
    json_arr=json_doc.array();//получаем из json_doc массив с клиентами
    while(1){//ищем клиента
        json_obj=json_arr.at(number).toObject();//выделяем очередной объект среди массива клиентов
        username=json_obj["nik"].toString();//выделяем имя из объекта (клиента)
        if(username.compare(name)==0){//нашли клиента
            flag_nik=0;//деактивируем флаг
            break;
        }
        number++;//идем к следующему объекту (клиенту)
    }
    if(flag_nik==0){
        json_file.close();//закрыли файл json
        //дальше идет редактирование числа попыток клиента в json, т.к. при вводе верного пароля они должны
        //сброситься в начальное состояние, т.е. в 3
        QFile json_file_edit(file_address);//снова задали файл json
        if(!json_file_edit.open(QIODevice::WriteOnly|QIODevice::Truncate)){//открыли этот файл только на запись
            //затерев все содержимое
            qWarning("Файл json не открылся");
            exit (3);
        }
        json_arr.removeAt(number);//удаляем объект, который надо редактировать в JsonDocument
        json_obj.remove("money");//удаляем поле, которое надо отредактировать в клиенте
        json_obj.insert("money",mes.toInt());//снова вставляем поле с начальным значение '3'
        json_file.close();//закрыли файл json

        //дальше идет редактирование числа попыток клиента в json, т.к. при вводе верного пароля они должны
        //сброситься в начальное состояние, т.е. в 3
        json_arr.insert(number,json_obj);//вставляем отредактированный объект в массив json
        QJsonDocument new_doc;//создаем новый JsonDocument, т.к. в старом хранятся оставшиеся
        //неотредактированные записи о пользователях
        new_doc.setArray(json_arr);//вносим отредактированный массив json в JsonDocument
        json_file_edit.write(new_doc.toJson());//записываем в файл json данные о пользователях
        json_file_edit.close();//закрыли файл json
    }
}
