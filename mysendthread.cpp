#include "mysendthread.h"
#include "myserialport.h"

MySendThread::MySendThread(QextSerialPort &adrPort,MySerialPort *mySerialPort)
    : port(adrPort),mySerialPort(mySerialPort)
{
    stopped = false;
}

MySendThread::~MySendThread()
{
    if (isRunning())
    {
        stopSending();
        wait();
    }
}

// Stop the sending operation
void MySendThread::stopSending()
{
    stopped = true;
}

void MySendThread::addDataToSend(const QByteArray &data)
{
    QMutexLocker locker(&mutexSend);
    for(int i=0;i<data.length();i++)
        dataToSend.enqueue(data);    
}

// Thread Send Loop
void MySendThread::run()
{
    QByteArray byteArray;
    forever
    {
        mutexSend.lock();
        if (dataToSend.isEmpty() || stopped)
        {
            mutexSend.unlock();
            stopped = false;
            break;
        }
        byteArray = dataToSend.dequeue();
        mutexSend.unlock();
        port.write(byteArray, 1);
    }
}
