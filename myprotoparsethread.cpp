#include "myprotoparsethread.h"
#include <QDebug>

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
    pkg->isValid = true;
}
//void MyProtoParseThread::parseFunction0(QByteArray &data,UartDataPackage *pkg)
//{
//    //pkg->head = pkg->tail = NULL;
//    pkg->head.clear();
//    pkg->tail.clear();
//    pkg->data = data;
//    pkg->length = data.length();
//    pkg->checksum = -1;
//    pkg->timestamp = QDateTime::currentDateTime();
//    pkg->isValid = true;
//}
//FFFE03A0B0C0
void MyProtoParseThread::parseFunction1(QByteArray &data,UartDataPackage *pkg)
{
    qDebug() << data.length();
    qDebug("%02x,%02x",data[0] & 0xFF,data[1]);
    if((data.length() != 6) || (data[0]&0xFF != 0xFF) || (data[1]&0xFF != 0xFE)){
        pkg->isValid = false;
        return;
    }
    pkg->isValid = true;
    int index = 0;
    pkg->head[0] = (data[index++] & 0xFF);
    pkg->head[1] = (data[index++] & 0xFF);
    pkg->length = (unsigned int)(data[index++] & 0xFF);
    for(int i=0;i<3;i++){
        pkg->data[i] = (data[index++] & 0xFF);
    }
    pkg->checksum = -1;
    pkg->timestamp = QDateTime::currentDateTime();
}
void MyProtoParseThread::parseFunction2(QByteArray &data,UartDataPackage *pkg)
{
    
}
