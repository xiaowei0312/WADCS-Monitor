#include "myprotoparsethread.h"
#include "myserialport.h"
#include <QDebug>
#include "stringutil.h"

MyProtoParseThread::MyProtoParseThread(QextSerialPort &adrPort,MySerialPort *mySerialPort)
    : port(adrPort),mySerialPort(mySerialPort)
{
    stopped = false;
    currentParsePos = 0;
    dataToParsed.clear();
}

MyProtoParseThread::~MyProtoParseThread()
{
    if (isRunning())
    {
        stopParsing();
        wait();
    }
}

// Stop the sending operation
void MyProtoParseThread::stopParsing()
{
    stopped = true;
}

void MyProtoParseThread::addDataToParsed(const QByteArray &data)
{
    QMutexLocker locker(&mutexParse);
    dataToParsed.append(data);
}

/*
 * struct UartProtoConfig{
    int headBytes;          //协议头字节个数
    int tailBytes;          //协议尾字节个数
    int length;             //协议长度字节个数
};
 */
void MyProtoParseThread::run()
{
    QByteArray byteArray;
    const UartProtoConfig &config = mySerialPort->getUartProtoConfig();
    forever
    {   
        //1. 获取要解析的数据
        mutexParse.lock();
        
        //1.1 判断是否有数据需要解析
        if(currentParsePos >= dataToParsed.length() || stopped) 
        {
            mutexParse.unlock();
            stopped = false;
            break;
        }
        
        //1.2 获取需要解析的数据
        byteArray = dataToParsed.mid(currentParsePos);
        
        //1.3 判断是否需要解析
        if(!config.needParsed) 
        {
            currentParsePos += byteArray.length();
            emit dataParsed(byteArray);
            mutexParse.unlock();
            break;
        }
        
        //1.4 计算头部
        qDebug("parsePos: %d\tlength: %d,%d",currentParsePos,dataToParsed.length(),byteArray.length());
        if(config.fixedHead.length()>0)
        {
            if(byteArray.length() < config.fixedHead.length() || stopped)   //数据长度 < 协议头长度
            {
                stopped = false;
                mutexParse.unlock();
                break;
            }
            if(!byteArray.startsWith(config.fixedHead))     //数据头 和 协议头 不一致         
            {
                currentParsePos++;
                mutexParse.unlock();
                continue;
            }
        }
        //1.5 截取包长
        int packageLength = config.fixedLength;
        if(!config.isFixedLength) //非固定长度协议
        {
            int baseLength = config.fixedHead.length() + config.fixedTail.length() 
                    + config.checksumBytes + config.lengthBytes;            
            if ((byteArray.length() < baseLength) || stopped)   //数据长度 < 协议基本长度（不包括数据）
            {
                stopped = false;
                mutexParse.unlock();
                break;
            }
            packageLength = baseLength;
            if(config.lengthBytes > 0)
            {    
                QByteArray lengthCol = byteArray.mid(config.fixedHead.length(),config.lengthBytes);
                packageLength = baseLength + StringUtil::convertByteArrayToInteger(lengthCol);
            }
        }
        if(byteArray.length() < packageLength)          //数据长度 < 协议长度（包括数据）
        {
            stopped = false;
            mutexParse.unlock();
            break;
        }
        byteArray = dataToParsed.mid(currentParsePos,packageLength);
        //1.6 计算尾部
        if(config.fixedTail.length()>0)
        {
            if(!byteArray.endsWith(config.fixedTail))            //数据尾 和 协议尾 不一致    
            {
                currentParsePos++;
                mutexParse.unlock();
                continue;
            }
        }
        //1.7 计算校验和
        //Fixed Me
        currentParsePos += packageLength;
        mutexParse.unlock();
        
        //2. 解析协议
        UartDataPackage pkg;
        parseData(byteArray,&pkg);
        emit dataParsed(pkg);
    }
}

void MyProtoParseThread::parseData(QByteArray &data,UartDataPackage *pkg)
{
    pkg->totalLength = data.length();
    //const UartProtoConfig &config = mySerialPort->getUartProtoConfig();
    parseFunction1(data,pkg);
}

//FFFE03A0B0C0
void MyProtoParseThread::parseFunction1(QByteArray &data,UartDataPackage *pkg)
{
    const UartProtoConfig &config = mySerialPort->getUartProtoConfig();
    pkg->head = config.fixedHead;
    pkg->tail = config.fixedTail;
    if(config.isFixedLength)
        pkg->length = config.fixedLength - config.fixedHead.length() - config.fixedTail.length() - config.checksumBytes
                -config.lengthBytes;    //  lengthBytes应该为0
    else
    {
        QByteArray lengthCol = data.mid(config.fixedHead.length(),config.lengthBytes);
        pkg->length = StringUtil::convertByteArrayToInteger(lengthCol);
    }
    pkg->data = data.mid(pkg->head.length()+config.lengthBytes,pkg->length);
    pkg->checksum = -1;
    pkg->timestamp = QDateTime::currentDateTime();
    pkg->isValid = true;
}
