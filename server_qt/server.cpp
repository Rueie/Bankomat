#include <QCoreApplication>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <map>
#include <list>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>

using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);


    QString file_address="/home/rueie/Qt_projects/1.1.server/server_qt/users.json";//адрес файла json с данными
    int server_socket,new_socket;//сокеты
    int opt=1;//вспомогательная переменная для подключения сокетов
    struct sockaddr_in address;//адрес сокета
    int addrlen=sizeof(address);//размер адреса сокета

    //пробуем подключиться к сокету
    if((server_socket=socket(AF_INET,SOCK_STREAM,0))==0){//если неудача, то ошибка в консоль
        cerr<<"<Ошибка сокета>";
        return 1;
    }

    //пробуем подключиться к порту
    if(setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR|SO_REUSEPORT,&opt,sizeof(opt))){//если неудача, то ошибка
        cerr<<"<Ошибка подключения порта>";
        return 2;
    }

    //параметры порта
    address.sin_family=AF_INET;
    address.sin_addr.s_addr=INADDR_ANY;
    address.sin_port=htons(8080);

    //вносим настройки на подключенный сокет
    if(bind(server_socket,(struct sockaddr*)&address,sizeof(address))<0){//если неудача, то ошибка
        cerr<<"<Ошибка внесения настроек в сокет>";
        return 3;
    }

    //ждем подключения
    if(listen(server_socket,3)<0){
        cerr<<"<Подключений больше, чем возможно>";
        return 4;
    }

    while(1){//общаемся с клиентом
        //подключили сокет для общения с клиентом
        if((new_socket=accept(server_socket,(struct sockaddr*)&address,(socklen_t*)&addrlen))<0){//если неудача, то ошибка
            cerr<<"<Ошибка подключения сокета для общения с клиентом>";
            return 5;
        }

        char buffer[1024]={0};//буфер для общения
        QFile json_file(file_address);//файл json с данными
        QString reader;//строка для чтения в неё файла json
        QJsonDocument json_doc;//для работы с файлом json
        QJsonArray json_arr;//для работы с массивами в json
        int number=0;//номер объекта в массиве json
        QJsonObject json_obj;//для работы с объектами json
        //элементы объекта в json
        QString name;//ник клиента
        QString status;//статус заблокирован(blocked)//активен(normal)
        QString password;//пароль
        int money;//деньги на счету

        int attempts;//кол-во попыток до блокировки 'карты'

        int flag_nik=0;//флаг того, что подключающийся человек был найден среди клиентов в json

        memset(buffer,0,sizeof(buffer));//чистим буфер на всякий крайний
        read(new_socket,buffer,1024);//читаем тип запроса от клиента
        if(buffer[0]=='s'){//запрос типа 's' от слова <Sign>
            send(new_socket,"+",1,0);//маякнули клиенту, что запрос на сервер был удачным
            memset(buffer,0,sizeof(buffer));//очистили буфер
            read(new_socket,buffer,1024);//считываем ник, который ввел пользователь
            cout<<"Пользователь с ником <"<<buffer<<"> пытается зайти на сервер\n";

            if(!json_file.exists()){//проверям доступность файла json
                cerr<<"Файл json не доступен"<<endl;
                return 6;
            }
            else if(!json_file.open(QIODevice::ReadOnly)){//открываем файл json только для чтения и проверяем, выполнилось ли открытие
                cerr<<"Ошибка открытия файла json"<<endl;
                return 7;
            }
            else{
                reader=json_file.readAll();//читаем весь файл с данными о клиентах
                json_doc=QJsonDocument::fromJson(reader.toUtf8());//формируем объект типо jsondocument
                json_arr=json_doc.array();//получаем из json_doc массив с клиентами\

                QString admin_exit="close";
                if(admin_exit.compare(buffer)==0){//закрытие сервера админом
                    cout<<"закрытие сервера\n";
                    return 0;
                }

                while(1){//ищем клиента
                    json_obj=json_arr.at(number).toObject();//выделяем очередной объект среди массива клиентов
                    if(json_obj.isEmpty()){//не нашли его, т.е. дошли до конца файла
                        cout<<"Пользователя <"<<buffer<<"> нет в базе данных\n";
                        cout<<"Отправление сообщения на клиент\n";//оно стоит в else к флагу
                        flag_nik=1;//активаируем флаг, 1=нет такого клиента, 0=есть
                        break;
                    }
                    name=json_obj["nik"].toString();//выделяем имя из объекта (клиента)
                    if(name.compare(buffer)==0){//нашли клиента
                        flag_nik=0;//деактивируем флаг
                        break;
                    }
                    number++;//идем к следующему объекту (клиенту)
                }
                if(flag_nik==0){//если нашли пользователя, то есть смысл дальше общаться с клиентом
                    send(new_socket,"+",1,0);//отправили сообщение клиенту о том, что пользователь был найден
                    cout<<"Пользователь <"<<buffer<<"> есть среди клиентов\n";
                    cout<<"Отправление сообщения на клиент\n";
                    status=json_obj["status"].toString();//считываем статус уже у найденного клиента
                    if(status.compare("blocked")!=0){//если статус 'заблокирован'(blocked), то сообщение в else, а так - работаем с клиентом дальше
                        send(new_socket,"+",1,0);//отправили сообщение о том, что пользователь не в бане
                        cout<<"Пользователь <"<<buffer<<"> не заблокирован\nОтправление сообщения на клиент\n";
                        attempts=json_obj["attempts"].toInt();//считываем число оставшихся попыток у пользователя до блокировки аккаунта
                        if(attempts>0){//если попытки еще остались, то идем дальше
                            cout<<"У пользователя <"<<buffer<<"> есть еще "<<attempts<<" попыток(ки/а) входа до блокировки\n";
                            cout<<"Отправление сообщения клиенту\n";
                            send(new_socket,"+",1,0);//отправили сообщение клиенту, что у пользователя еще есть попытки
                            password=json_obj["password"].toString();//считываем пароль клиента из json
                            memset(buffer,0,sizeof(buffer));//чистим буфер
                            read(new_socket,buffer,1024);//читаем пароль, который ввел клиента в 'банкомат'
                            if(password.compare(buffer)==0){//если пароль верный, то идем дальше
                                send(new_socket,"+",1,0);//маякнули клиенту, что пароль верный
                                cout<<"Клиент с именем <"<<name.toStdString()<<"> ввел верный пароль\n";
                                cout<<"Отправляем сообщение на клиент\n";
                                money=json_obj["money"].toInt();//считываем счет клиента из объекта найденного пользователя
                                //преобразование числа в строку
                                QString buffer_for_money=QString::number(money);
                                //затем в массив из char
                                char *buffer_out;
                                QByteArray str=buffer_for_money.toLocal8Bit();
                                buffer_out=str.data();
                                //------------------------------конец преобразования в массив char
                                send(new_socket,buffer_out,sizeof(buffer_out),0);//отправили счет на клиент
                                json_file.close();//закрыли файл json
                                //дальше идет редактирование числа попыток клиента в json, т.к. при вводе верного пароля они должны
                                //сброситься в начальное состояние, т.е. в 3
                                QFile json_file_edit(file_address);//снова задали файл json
                                if(!json_file_edit.open(QIODevice::WriteOnly|QIODevice::Truncate)){//открыли этот файл только на запись
                                    //затерев все содержимое
                                    qWarning("Файл json не открылся");
                                    return 8;
                                }
                                json_arr.removeAt(number);//удаляем объект, который надо редактировать в JsonDocument
                                json_obj.remove("attempts");//удаляем поле, которое надо отредактировать в клиенте
                                json_obj.insert("attempts",3);//снова вставляем поле с начальным значение '3'
                                json_arr.insert(number,json_obj);//вставляем отредактированный объект в массив json
                                QJsonDocument new_doc;//создаем новый JsonDocument, т.к. в старом хранятся оставшиеся
                                //неотредактированные записи о пользователях
                                new_doc.setArray(json_arr);//вносим отредактированный массив json в JsonDocument
                                json_file_edit.write(new_doc.toJson());//записываем в файл json данные о пользователях
                                json_file_edit.close();//закрыли файл json
                                cout<<"Операция входа в личный кабинет была успешна\n";
                            }
                            else{//случай, когда пароль неудачен
                                cout<<"Клиент с именем <"<<name.toStdString()<<"> ввел неверный пароль\n";
                                cout<<"Отправление сообщения на клиент\n";
                                send(new_socket,"-",1,0);//шлем отрицательный результат попытки ввода на клиент
                                //абсолютная аналогия со случаем с верным паролем, только тут мы убавляем число попыток на 1
                                json_file.close();
                                QFile json_file_edit(file_address);
                                if(!json_file_edit.open(QIODevice::WriteOnly|QIODevice::Truncate)){
                                    qWarning("Файл json не открылся");
                                    return 8;
                                }
                                json_arr.removeAt(number);
                                json_obj.remove("attempts");
                                json_obj.insert("attempts",attempts-1);
                                json_arr.insert(number,json_obj);
                                QJsonDocument new_doc;
                                new_doc.setArray(json_arr);
                                json_file_edit.write(new_doc.toJson());
                                json_file_edit.close();
                            }
                        }
                        else{//случай, когда попыток не остолсь
                            cout<<"У клиента <"<<name.toStdString()<<"> больше не осталость попыток на вход\n";
                            cout<<"Блокирование клиента с именем <"<<name.toStdString()<<">\n";
                            cout<<"Отправление сообщения на клиент\n";
                            send(new_socket,"-",1,0);//отправили сообщение клиенту, что попыток больше нет
                            //дальше абсолютная аналогия cо случаем с верным паролем, только тут мы статус меням на 'заблокирован',
                            //а число попыток ставим в начальное состояние, т.е. в '3'
                            json_file.close();
                            QFile json_file_edit(file_address);
                            if(!json_file_edit.open(QIODevice::WriteOnly|QIODevice::Truncate)){
                                qWarning("Файл json не открылся");
                                return 8;
                            }
                            json_arr.removeAt(number);
                            json_obj.remove("status");
                            json_obj.insert("status","blocked");
                            json_obj.remove("attempts");
                            json_obj.insert("attempts",3);
                            json_arr.insert(number,json_obj);
                            QJsonDocument new_doc;
                            new_doc.setArray(json_arr);
                            json_file_edit.write(new_doc.toJson());
                            json_file_edit.close();
                        }
                    }
                    else {//если клиент заблокирован
                        send(new_socket,"-",1,0);//маякнули клиенту, что пользователь заблокирован
                        cout<<"Клиент с именем <"<<buffer<<"> заблокирован\n";
                        cout<<"Отправляем сообщение на клиент\n";
                    }
                }
                else send(new_socket,"-",1,0);//про это сообщение говорилось в while, сообщение о неудаче поиска клиента
            }

            json_file.close();//закрытие файла json
        }
        else if(buffer[0]=='m'){//запрос типа 'm' от слова 'money', здесь изменяется счет пользователя

            send(new_socket,"+",1,0);//маякнули, что запрос удачен

            memset(buffer,0,sizeof(buffer));//чистим буфер
            read(new_socket,buffer,1024);//пришел ник пользователя
            cout<<"Пользователь с ником <"<<buffer<<"> работает в офисе\n";

            if(!json_file.exists()){//проверка на доступность файла json
                cerr<<"Файл json не доступен"<<endl;
                return 6;
            }
            else if(!json_file.open(QIODevice::ReadOnly)){//проверка на то, открыля ли файл json
                cerr<<"Ошибка открытия файла json"<<endl;
                return 7;
            }
            else{
                //куча проверок тут не нужна, т.к. клиент уже прошел авторизацию
                reader=json_file.readAll();//читаем весь
                json_doc=QJsonDocument::fromJson(reader.toUtf8());//преобразуем его в JsonDocument
                json_arr=json_doc.array();//вытаскиваем из json_doc массив с клиентами
                while(1){//ищем клиента
                    json_obj=json_arr.at(number).toObject();//вытащили клиента из массива клиентов
                    if(json_obj.isEmpty()){//не нашли пользователя, т.е. дошли до конца файла
                        cout<<"Пользователя <"<<buffer<<"> нет в базе данных\n";
                        cout<<"Отправление сообщения на клиент\n";
                        flag_nik=1;//активируем флаг того, что пользователь не найден
                        break;
                    }
                    name=json_obj["nik"].toString();//смотрим имя пользователя в json
                    if(name.compare(buffer)==0){//нашли клиента
                        flag_nik=0;//деактивируем флаг
                        break;
                    }
                    number++;//переходим на следующего клиента
                }
                if(flag_nik==0){//пользователь был найден
                    cout<<"Пользователь <"<<buffer<<"> есть среди клиентов\n";
                    cout<<"Отправляем сообщение на клиент\n";
                    send(new_socket,"+",1,0);//маякнули клиенту, что нашли пользователя
                    memset(buffer,0,sizeof(buffer));//чистим буфер
                    read(new_socket,buffer,1024);//получаем новый (уже посчитанный) счет пользователя от клиента
                    //преобразование char в Qstring
                    QString *money_string=new QString(buffer);
                    money_string->truncate(sizeof(buffer));
                    //--------------------------конец преобразования
                    json_file.close();//закрыли файл json
                    //дальше полная аналогия, как в запросе на авторизацию для случая верного пароля, только здесь мы
                    //просто редактируем счет
                    QFile json_file_edit(file_address);
                    if(!json_file_edit.open(QIODevice::WriteOnly|QIODevice::Truncate)){
                        qWarning("Файл json не открылся");
                        return 8;
                    }
                    json_arr.removeAt(number);
                    json_obj.remove("money");
                    json_obj.insert("money",money_string->toInt());
                    json_arr.insert(number,json_obj);
                    QJsonDocument new_doc;
                    new_doc.setArray(json_arr);
                    json_file_edit.write(new_doc.toJson());
                    json_file_edit.close();
                    send(new_socket,"+",1,0);//маякнули клиенту, что операция прошла успешна
                    cout<<"Операция у пользователя <"<<name.toStdString()<<"> прошла успешно\n";
                    cout<<"Отправление сообщения на клиент\n";
                }
                else{
                    cout<<"Пользователь с нимком <"<<buffer<<"> не был найден";
                    cout<<"Отправление сообщения на клиент\n";
                    send(new_socket,"-",1,0);//маякнули клиенту, что пользователь не был найден
                }
            }
        }
        else {
            send(new_socket,"-",1,0);//маякнули клиенту, что сервер не знает команду такого типа
            cerr<<"Пришла неизвестная команда серверу\n";
        }
    }

    return a.exec();
}
