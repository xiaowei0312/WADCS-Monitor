#include "myprotoparsethread.h"

MyProtoParseThread::MyProtoParseThread(QextSerialPort &adrPort): port(adrPort)
{
    dataToParse.clear();
    stopped = false;
    parseFunctionIndex = 0; //缺省使用0号解析函数
}

MyProtoParseThread::~MyProtoParseThread()
{
    if (isRunning())
    {
        stopParsing();
        wait();
    }
}

void MyProtoParseThread::setParseFunction(int index)
{
    parseFunctionIndex = index;
}

void MyProtoParseThread::setParseFunctionCmd(QString &cmd)
{
    parseFunctionCmd = cmd;
}

// Add the data to the Send Queue
void MyProtoParseThread::addDataToParse(const QByteArray &dataToAdd)
{
    QMutexLocker locker(&mutexParse);
    dataToParse.enqueue(dataToAdd);
    if (!isRunning())
        start();
}
// Stop the sending operation
void MyProtoParseThread::stopParsing()
{
    stopped = true;
}
// Thread Send Loop
void MyProtoParseThread::run()
{
    QByteArray byteArray;
    forever
    {
        mutexParse.lock();
        if (dataToParse.isEmpty() || stopped)
        {
            mutexParse.unlock();
            stopped = false;
            break;
        }
        byteArray = dataToParse.dequeue();
        mutexParse.unlock();
        
        UartDataPackage pkg;
        parseData(byteArray,&pkg);
        emit dataParsed(pkg);
    }
}

void MyProtoParseThread::parseData(QByteArray &data,UartDataPackage *pkg)
{
    switch(parseFunctionIndex)
    {
    case -1:    //调用外部命令
        parseFunctionByExternalPlugin(data,pkg);
        break;
    case 0:     //调用0号解析函数
        parseFunction0(data,pkg);
        break;  
    case 1:     //调用1号解析函数
        parseFunction1(data,pkg);
        break;
    case 2:     //调用1号解析函数
        parseFunction2(data,pkg);
        break;
    }
}

void MyProtoParseThread::parseFunctionByExternalPlugin(QByteArray &data, UartDataPackage *pkg)
{
    
}
void MyProtoParseThread::parseFunction0(QByteArray &data,UartDataPackage *pkg)
{
    //pkg->head = pkg->tail = NULL;
    pkg->head.clear();
    pkg->tail.clear();
    pkg->data = data;
    pkg->length = data.length();
    pkg->checksum = -1;
    pkg->timestamp = QDateTime::currentDateTime();
}
void MyProtoParseThread::parseFunction1(QByteArray &data,UartDataPackage *pkg)
{
    
}
void MyProtoParseThread::parseFunction2(QByteArray &data,UartDataPackage *pkg)
{
    
}
