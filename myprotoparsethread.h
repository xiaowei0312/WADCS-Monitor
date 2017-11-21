#ifndef MYPROTOPARSETHREAD_H
#define MYPROTOPARSETHREAD_H

#include <QThread>
#include <QQueue>
#include "qextserialport.h"
#include "common.h"

class MyProtoParseThread : public QThread
{
    Q_OBJECT
public:
    MyProtoParseThread(QextSerialPort &adrPort);
    ~MyProtoParseThread();
    void addDataToParse(const QByteArray &dataToAdd);
    void stopParsing();
    void setParseFunction(int index);
    void setParseFunctionCmd(QString &cmd);
private: 
    void parseData(QByteArray &data,UartDataPackage *pkg);
    void parseFunctionByExternalPlugin(QByteArray &data,UartDataPackage *pkg);
    void parseFunction0(QByteArray &data,UartDataPackage *pkg);
    void parseFunction1(QByteArray &data,UartDataPackage *pkg);
    void parseFunction2(QByteArray &data,UartDataPackage *pkg);
protected:
    void run();
signals:
    void dataParsed(const UartDataPackage &parsedPkg);
private :
    QextSerialPort &port;
    QQueue<QByteArray> dataToParse;
    QMutex mutexParse;
    bool stopped;
    int parseFunctionIndex; //解析函数索引 >=0 代表引用本地解析函数， <0代表使用parseFunctionCmd指定的插件进行解析
    QString parseFunctionCmd;   //解析命令（由外部传入），配合 parseFunctionIndex<0使用
};

#endif // MYPROTOPARSETHREAD_H
