#ifndef MYRECEIVETHREAD_H
#define MYRECEIVETHREAD_H

#include <QThread>
#include "qextserialport.h"

class MyReceiveThread : public QThread
{
    Q_OBJECT
public:
    MyReceiveThread(QextSerialPort &adrPort);
    ~MyReceiveThread();
    void stopReceiving();
protected:
    void run();
signals:
    void dataReceived(const QByteArray &dataReceived);
private :
    QextSerialPort &port;
    QMutex mutexReceive;
    bool stopped;
};

#endif // MYRECEIVETHREAD_H
