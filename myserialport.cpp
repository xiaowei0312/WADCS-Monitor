#include <QDebug>
#include "myserialport.h"
#include "myreceivethread.h"
#include "mysendthread.h"
#include "myprotoparsethread.h"

MySerialPort::MySerialPort(QObject *parent) :
    QObject(parent)
{
    initPrivateVariable();
}

//MySerialPort::MySerialPort(const QString &name, const BaudRateType baudRate, const DataBitsType dataBits,
//                           const ParityType parity, const StopBitsType stopBits, const FlowType flowControl,
//                           ulong seconds, ulong milliseconds)
//{
//    initPrivateVariable();
//    setPortName(name);
//    setBaudRate(baudRate);
//    setDataBits(dataBits);
//    setParity(parity);
//    setStopBits(stopBits);
//    setFlowControl(flowControl);
//    setTimeout(seconds, milliseconds);
//}

MySerialPort::~MySerialPort()
{
    if (sendThread)
    {
        delete sendThread;
        sendThread = NULL;
    }
    if (receiveThread)
    {
        delete receiveThread;
        receiveThread = NULL;
    }
    if (parseThread)
    {
        delete parseThread;
        parseThread = NULL;
    }
    if (isOpen())
        port.close();
}

QByteArray MySerialPort::receiveBuffer()
{
    QMutexLocker locker(&mutexSerialPort);
    return this->hasReceivedData;
}

void MySerialPort::clearRecvBuffer()
{
    QMutexLocker locker(&mutexSerialPort);
    this->hasReceivedData.clear();
}

void MySerialPort::pushDataToRecvBuffer(const QByteArray &data)
{
    QMutexLocker locker(&mutexSerialPort);
    this->hasReceivedData.append(data);
}

bool MySerialPort::setUartProtoConfig(const UartProtoConfig &config,QString &errorText)
{
    
//    qDebug() << this->config.needParsed << ' ' << this->config.isFixedLength << ' '
//        << config.fixedLength << ' ' << config.fixedHead << ' ' << config.fixedTail << ' ' << config.lengthBytes
//        << ' ' << config.checksumBytes << ' ' << config.checksum;
    if(validUartProtoConfig(config,errorText))
    {
        this->config = config;
        return true;
    }
    return false;
}

bool MySerialPort::validUartProtoConfig(const UartProtoConfig & config,QString &errorText)
{
    if(config.isFixedLength && config.fixedLength == 0)
    {
        errorText = QString::fromLocal8Bit("定长协议长度必须大于0");
        return false;
    }
    if(config.isFixedLength && config.lengthBytes>0)
    {
        errorText = QString::fromLocal8Bit("定长协议无须指定长度字节");
        return false;
    }
    if(!config.isFixedLength && config.lengthBytes == 0)
    {
        errorText = QString::fromLocal8Bit("非定长协议长度字节不能为0");
        return false;
    }
    if(config.isFixedLength && 
            config.fixedLength < config.fixedHead.length() + config.fixedTail.length() + config.checksumBytes)
    {
        errorText = QString::fromLocal8Bit("定长协议长度不能小于[头+尾+校验]字节长度之和");
        return false;
    }
    return true;
}
UartProtoConfig &MySerialPort::getUartProtoConfig()
{
    return config;   
}


void MySerialPort::initPrivateVariable()
{
    // Init private variable
    sendThread = NULL;
    receiveThread = NULL;
    parseThread = NULL;
    sendingEnable = false;
    receivingEnable = false;
    parsingEnable = false;
    closeCalled = false;
    
    saveStateSendingEnable = false;
    saveStateReceivingEnable = false;
    saveStateParsingEnable = false;
    saveStateReceiveData = false;
    saveStateParseData = false;
    
    config.needParsed = false;
    
    clearRecvBuffer();
}

// Open the SerialPort
bool MySerialPort::open()
{
    bool res = port.open(QIODevice::ReadWrite);
    // If the port is reopened after an earlier closure restores the previous state
    if (closeCalled)
    {
        if (saveStateSendingEnable)
            enableSending();
        if (saveStateReceivingEnable)
            enableReceiving();
        if (saveStateParsingEnable)
            enableParsing();
        if (saveStateReceiveData)
            receiveData();
        closeCalled = false;
    }
    return res;
}
bool MySerialPort::open(const QString &name, const BaudRateType baudRate, const DataBitsType dataBits,
                        const ParityType parity, const StopBitsType stopBits, const FlowType flowControl,
                        ulong seconds, ulong milliseconds)
{
    QString portName = name;
    if(portName.size() > QString("ComX").size())    //处理>10的情况
        portName = "\\\\.\\" + name;
    setPortName(portName);
    if(port.open(QIODevice::ReadWrite)){
        setBaudRate(baudRate);
        setDataBits(dataBits);
        setParity(parity);
        setStopBits(stopBits);
        setFlowControl(flowControl);
        setTimeout(seconds, milliseconds);
        return true;
    }
    return false;
}

// SerialPort is open?
bool MySerialPort::isOpen() const
{
    return port.isOpen();
}
// Close the SerialPort
void MySerialPort::close()
{
    closeCalled = true;
    // Save the state
    saveStateSendingEnable = isSendingEnable();
    saveStateReceivingEnable = isReceivingEnable();
    saveStateParsingEnable = isParsingEnable();
    // Close the port
    disableSending();
    disableReceiving();
    disableParsing();
    port.close();
    // TODO: should I stop send and receive?
}

// Setter and getter for the basic property of the QextSerialPort
void MySerialPort::setPortName(const QString &name)
{
    port.setPortName(name);
}
QString MySerialPort::portName() const
{
    return port.portName();
}
void MySerialPort::setBaudRate(const BaudRateType baudRate)
{
    port.setBaudRate(baudRate);
}
BaudRateType MySerialPort::baudRate() const
{
    return port.baudRate();
}
void MySerialPort::setDataBits(const DataBitsType dataBits)
{
    port.setDataBits(dataBits);
}
DataBitsType MySerialPort::dataBits() const
{
    return port.dataBits();
}
void MySerialPort::setParity(const ParityType parity)
{
    port.setParity(parity);
}
ParityType MySerialPort::parity() const
{
    return port.parity();
}
void MySerialPort::setStopBits(StopBitsType stopBits)
{
    port.setStopBits(stopBits);
}
StopBitsType MySerialPort::stopBits() const
{
    return port.stopBits();
}
void MySerialPort::setFlowControl(const FlowType flowControl)
{
    port.setFlowControl(flowControl);
}
FlowType MySerialPort::flowControl() const
{
    return port.flowControl();
}
void MySerialPort::setTimeout(const ulong seconds, const ulong milliseconds)
{
    //port.setTimeout(seconds, milliseconds);
    port.setTimeout(milliseconds);
}
// Enable the SerialPort to send data (init the thread)
void MySerialPort::enableSending()
{
    // If the Sending is not already active AND the sendThead is not initialized
    if (!sendingEnable && !sendThread)
    {
        sendThread = new MySendThread(port,this);
        sendingEnable = true;
    }
}
// Disable the SerialPort to send data (terminate the thread)
void MySerialPort::disableSending()
{
    // If the Sending is already active AND there is a sendThread
    if (sendingEnable && sendThread)
    {
        delete sendThread;
        sendThread = NULL;
        sendingEnable = false;
    }
}
bool MySerialPort::isSendingEnable() const
{
    return sendingEnable;
}
// Stop the currently sending data operation (don't terminate the thread)
void MySerialPort::stopSending()
{
    // If the Sending is not alread active OR the sendThread is not initialized
    if (!sendingEnable || !sendThread)
        return;
    // If the SerialPort is currently sending data, stop
    if (sendThread->isRunning())
    {
        sendThread->stopSending();
        //wait(); ??????????
        sendThread->wait();
    }
}
// Enqueue data to the sendThread queue
// return 1     OK
// return 2     port closed
// return 3     sending operation disable
uchar MySerialPort::sendData(const QByteArray &data)
{
    // check the port if is open
    if (!isOpen())
        return 2;
    // check if sending operation is enable
    if (!sendingEnable || !sendThread)
        return 3;
    
    sendThread->addDataToSend(data);
    if (!sendThread->isRunning())
        sendThread->start();
    return 1;
}

// Enable the SerialPort to receive data (init the thread)
void MySerialPort::enableReceiving()
{
    // If the Receiving is not already active AND the receiveThead is not initialized
    if (!receivingEnable && !receiveThread)
    {
        receiveThread = new MyReceiveThread(port,this);
        connect(receiveThread, SIGNAL(dataReceived(const QByteArray &)),
                this, SLOT(onDataReceived(const QByteArray &)));
        receivingEnable = true;
    }
}
// Disable the SerialPort to receive data (terminate the thread)
void MySerialPort::disableReceiving()
{
     // If the Receiving is already active AND there is a receiveThread
    if (receivingEnable && receiveThread)
    {
        delete receiveThread;
        receiveThread = NULL;
        receivingEnable = false;
    }
}
bool MySerialPort::isReceivingEnable() const
{
    return receivingEnable;
}
// Stop the currently receiving data operation (don't terminate the thread)
void MySerialPort::stopReceiving()
{
        // If the Receiving is not alread active OR the receiveThread is not initialized
    if (!receivingEnable || !receiveThread)
        return;
    // If the SerialPort is currently receiving data, stop
    if (receiveThread->isRunning())
    {
        saveStateReceiveData = false;
        receiveThread->stopReceiving();
        // wait();
        receiveThread->wait();
    }
}
// Start the receiving thread
// return 1     OK
// return 2     port closed
// return 3     receiving operation disable
uchar MySerialPort::receiveData()
{
    // check the port if is open
    if (!isOpen())
        return 2;
    // check if receiving operation is enable
    if (!receivingEnable && !receiveThread)
        return 3;
    // If the SerialPort receiving thread is not currently active, start it
    if (!receiveThread->isRunning())
    {
        saveStateReceiveData = true;
        receiveThread->start();
    }
    return 1;
}

// Enable the SerialPort to parse data (init the thread)
void MySerialPort::enableParsing()
{
    // If the Sending is not already active AND the sendThead is not initialized
    if (!parsingEnable && !parseThread)
    {
        parseThread = new MyProtoParseThread(port,this);
        connect(parseThread, SIGNAL(dataParsed(const UartDataPackage&)),
                this, SLOT(onDataParsed(const UartDataPackage &)));
        connect(parseThread, SIGNAL(dataParsed(const QByteArray &)),
                this, SLOT(onDataParsed(const QByteArray &)));
        parsingEnable = true;
    }
}
// Disable the SerialPort to send data (terminate the thread)
void MySerialPort::disableParsing()
{
    // If the Sending is already active AND there is a sendThread
    if (parsingEnable && parseThread)
    {
        delete parseThread;
        parseThread = NULL;
        parsingEnable = false;
    }
}
bool MySerialPort::isParsingEnable() const
{
    return parsingEnable;
}
// Stop the currently sending data operation (don't terminate the thread)
void MySerialPort::stopParsing()
{
    // If the Sending is not alread active OR the sendThread is not initialized
    if (!parsingEnable || !parseThread)
        return;
    // If the SerialPort is currently sending data, stop
    if (parseThread->isRunning())
    {
        saveStateParseData = false;
        parseThread->stopParsing();
        parseThread->wait();
    }
}
// Enqueue data to the sendThread queue
// return 1     OK
// return 2     port closed
// return 3     sending operation disable
uchar MySerialPort::parseData(const QByteArray &data)
{
    // check the port if is open
    if (!isOpen())
        return 2;
    // check if sending operation is enable
    if (!parsingEnable || !parseThread)
        return 3;
    parseThread->addDataToParsed(data);
    if (!parseThread->isRunning())
        parseThread->start();
    return 1;
}

void MySerialPort::onDataReceived(const QByteArray &data)
{
    pushDataToRecvBuffer(data);
    emit dataReceived(data);
    parseData(data);
}

void MySerialPort::onDataParsed(const UartDataPackage &parsedPkg)
{
    emit dataParsed(parsedPkg);
}

void MySerialPort::onDataParsed(const QByteArray &data)
{
    emit dataParsed(data);
}
