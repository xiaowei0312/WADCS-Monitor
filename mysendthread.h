#ifndef MYSENDTHREAD_H
#define MYSENDTHREAD_H

#include <QThread>
#include <QQueue>
#include "qextserialport.h"

class MySendThread : public QThread
{
    Q_OBJECT
public:
    MySendThread(QextSerialPort &adrPort);
    ~MySendThread();
    void addDataToSend(const QByteArray &dataToAdd);
    void stopSending();
protected:
    void run();
private:
    QextSerialPort &port;
    QQueue<QByteArray> dataToSend;
    QMutex mutexSend;
    bool stopped;
};

#endif // MYSENDTHREAD_H
