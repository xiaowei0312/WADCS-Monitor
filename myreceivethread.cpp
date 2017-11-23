#include "myreceivethread.h"
#include <QDebug>

MyReceiveThread::MyReceiveThread(QextSerialPort &adrPort) : port(adrPort)
{
    stopped = false;
}

MyReceiveThread::~MyReceiveThread()
{
    if (isRunning())
    {
        stopReceiving();
        wait();
    }
}

// Stop the sending operation
void MyReceiveThread::stopReceiving()
{
    stopped = true;
}
// Thread Receive Loop
void MyReceiveThread::run()
{
    int numBytes = 0;
    QByteArray data;
    forever
    {
        if (stopped)
        {
            stopped = false;
            break;
        }
        numBytes = port.bytesAvailable();
        if (numBytes>0)
        {
            mutexReceive.lock();
            data = port.read(numBytes);
            mutexReceive.unlock();
            emit dataReceived(data);
        }
    }
}
