#ifndef MYRECEIVETHREAD_H
#define MYRECEIVETHREAD_H

#include <QThread>
#include "qextserialport.h"

class MySerialPort;
class MyReceiveThread : public QThread
{
    Q_OBJECT
public:
    MyReceiveThread(QextSerialPort &adrPort,MySerialPort *mySerialPort);
    ~MyReceiveThread();
    void stopReceiving();
protected:
    void run();
signals:
    void dataReceived(const QByteArray &dataReceived);
private :
    QextSerialPort &port;
    MySerialPort *mySerialPort;
    QMutex mutexReceive;
    bool stopped;
};

#endif // MYRECEIVETHREAD_H
