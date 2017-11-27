#ifndef MYPROTOPARSETHREAD_H
#define MYPROTOPARSETHREAD_H

#include <QThread>
#include <QQueue>
#include "qextserialport.h"
#include "common.h"

class MySerialPort;
class MyProtoParseThread : public QThread
{
    Q_OBJECT
public:
    MyProtoParseThread(QextSerialPort &adrPort,MySerialPort *mySerialPort);
    ~MyProtoParseThread();
    void stopParsing();
    void addDataToParsed(const QByteArray &data);
private: 
    void parseData(QByteArray &data,UartDataPackage *pkg);
    void parseFunction0(QByteArray &data,UartDataPackage *pkg);
    void parseFunction1(QByteArray &data,UartDataPackage *pkg);
protected:
    void run();
signals:
    void dataParsed(const UartDataPackage &parsedPkg);
    void dataParsed(const QByteArray &data);
private :
    QextSerialPort &port;
    MySerialPort *mySerialPort;
    QMutex mutexParse;
    bool stopped;
    
    QByteArray dataToParsed;
    int currentParsePos;
};

#endif // MYPROTOPARSETHREAD_H
