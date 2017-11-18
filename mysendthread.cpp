#include "mysendthread.h"

MySendThread::MySendThread(QextSerialPort &adrPort): port(adrPort)
{
    dataToSend.clear();
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

// Add the data to the Send Queue
void MySendThread::addDataToSend(const QByteArray &dataToAdd)
{
    QMutexLocker locker(&mutexSend);
    for (int i=0; i < dataToAdd.size(); i++)
        dataToSend.enqueue(QByteArray(1,dataToAdd.at(i)));
    if (!isRunning())
        start();
}
// Stop the sending operation
void MySendThread::stopSending()
{
    stopped = true;
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
