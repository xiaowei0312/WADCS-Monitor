#ifndef MYSENDTHREAD_H
#define MYSENDTHREAD_H

#include <QThread>
#include <QQueue>
#include "qextserialport.h"

class MySerialPort;
class MySendThread : public QThread
{
    Q_OBJECT
public:
    MySendThread(QextSerialPort &adrPort,MySerialPort *mySerialPort);
    ~MySendThread();
    void stopSending();
    void addDataToSend(const QByteArray &data);
protected:
    void run();
private:
    QextSerialPort &port;
    MySerialPort *mySerialPort;
    QMutex mutexSend;
    bool stopped;
    QQueue<QByteArray> dataToSend;
};

#endif // MYSENDTHREAD_H
