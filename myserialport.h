#ifndef MYSERIALPORT_H
#define MYSERIALPORT_H

#include <QObject>
#include <QVariant>
#include "qextserialport.h"
#include "common.h"

class MySendThread;
class MyReceiveThread;
class MyProtoParseThread;

Q_DECLARE_METATYPE(BaudRateType);
Q_DECLARE_METATYPE(DataBitsType);
Q_DECLARE_METATYPE(ParityType);
Q_DECLARE_METATYPE(StopBitsType);
Q_DECLARE_METATYPE(FlowType);

class MySerialPort : public QObject
{
    Q_OBJECT
private:
    QMutex mutexSerialPort;
    QextSerialPort port;
    MySendThread *sendThread;
    MyReceiveThread *receiveThread;
    MyProtoParseThread *parseThread;
    
    bool sendingEnable;
    bool receivingEnable;
    bool parsingEnable;
    // Variables to restore the previous state to a reopening of the SerialPort
    bool closeCalled;
    bool saveStateSendingEnable;
    bool saveStateReceivingEnable;
    bool saveStateParsingEnable;
    bool saveStateReceiveData;
    bool saveStateParseData;
public:
    QByteArray hasReceivedData;
    UartProtoConfig config;
private:
    void initPrivateVariable();
    
public:
    QByteArray receiveBuffer();
    void clearRecvBuffer();
    void pushDataToRecvBuffer(const QByteArray &data);
    
    void setUartProtoConfig(bool needParsed,int fixedLength,QByteArray fixedHead,QByteArray fixedTail,
            int lengthbytes,int checksumbytes);
    void setUartProtoConfig(const UartProtoConfig &config);
    UartProtoConfig &getUartProtoConfig();
    
    MySerialPort(QObject *parent = 0);
//    MySerialPort(const QString &name, const BaudRateType baudRate, const DataBitsType dataBits,
//                 const ParityType parity, const StopBitsType stopBits, const FlowType flowControl,
//                 ulong seconds = 0, ulong milliseconds = 10);
    ~MySerialPort();
    
    bool open();
    bool open(const QString &name, const BaudRateType baudRate, const DataBitsType dataBits,
              const ParityType parity, const StopBitsType stopBits, const FlowType flowControl,
              ulong seconds = 0, ulong milliseconds = 10);
    bool isOpen() const;
    void close();
    
    // Setter and getter for the basic property of the QextSerialPort
    void setPortName(const QString &name);
    QString portName() const;
    void setBaudRate(const BaudRateType baudRate);
    BaudRateType baudRate() const;
    void setDataBits(const DataBitsType dataBits);
    DataBitsType dataBits() const;
    void setParity(const ParityType parity);
    ParityType parity() const;
    void setStopBits(StopBitsType stopBits);
    StopBitsType stopBits() const;
    void setFlowControl(const FlowType flowControl);
    FlowType flowControl() const;
    void setTimeout(const ulong seconds, const ulong milliseconds);
    
    void enableSending();           // enable the SerialPort to send data (init the thread)
    void disableSending();          // disable the SerialPort to send data (terminate the thread)
    bool isSendingEnable() const;
    void stopSending();             // stop the currently sending data operation (don't terminate the thread)
    uchar sendData(const QByteArray &data); // send data to the SerialPort (enqueue data to the sendThread queue)
    // return 1     OK
    // return 2     port not open
    // return 3     sending operation disable
    
    void enableReceiving();         // enable the SerialPort to receive data (init the thread)
    void disableReceiving();        // disable the SerialPort to receive data (terminate the thread)
    bool isReceivingEnable() const;
    void stopReceiving();           // stop the currently receiving data operation (don't terminate the thread)
    uchar receiveData();                    // Start the receiving thread
    // return 1     OK
    // return 2     port closed
    // return 3     receiving operation disable
    
    void enableParsing();         // enable the SerialPort to parse data (init the thread)
    void disableParsing();        // disable the SerialPort to parse data (terminate the thread)
    bool isParsingEnable() const;
    void stopParsing();           // stop the currently parsing data operation (don't terminate the thread)
    uchar parseData(const QByteArray &data);    // parse data (enqueue data to the parseThread queue)
    // return 1     OK
    // return 2     port closed
    // return 3     parsing operation disable
signals:
    void dataReceived(const QByteArray &dataReceived);
    void dataParsed(const UartDataPackage &parsedPkg);
    void dataParsed(const QByteArray &data);
public slots:
    void onDataReceived(const QByteArray &dataReceived);
    void onDataParsed(const UartDataPackage &parsedPkg);
    void onDataParsed(const QByteArray &data);
};

#endif // MYSERIALPORT_H
