#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <windows.h>
#include <QInputDialog>
#include <QList>
#include "myserialport.h"
#include "qcustomplot.h"
#include "common.h"

namespace Ui {
class MainWindow;
}

class QextSerialPort;
class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
protected:
    void changeEvent(QEvent *e);
private:
    Ui::MainWindow *ui;
    
    //串口枚举相关变量
    wchar_t subkey[80];          
    wchar_t keyname[256]; //键名数组        
    char keyvalue[256];  //键值数组         
    int indexnum;          
    DWORD keysize,type,valuesize;         
    HKEY hKey; 
    
    //串口配置参数相关变量
    QList<QString> uartPortList;
    QList<QString> buadrateList;
    QList<QString> databitList;
    QList<QString> stopbitList;
    int currentPort;
    int currentBuadrate;
    int currentDataBit;
    int currentStopBit;
    int currentParity;
    int currentFlowControl;
    int currentTimeout;
    
    int currentUartSendHex;
    int currentUartSendNewLine;
    int currentUartSendTimely;
    int currentUartSendTimelyValue;
    
    int currentUartRecvHex;
    int currentUartAddTimeStamp;
    
    UartProtoConfig config;
    
    //串口操作相关变量
    MySerialPort *mySerialPort;
    QList<UartDataPackage> hasParsedData;
    
    //串口"定时发送"定时器
    QTimer *sendTimer;
private:
    //ui初始化
    void uartSettingsInit();
    void uartParseInit();
    void uartEnumation();
    void initWidgets();
    void appendStringToPlainText(QString data);
    QString getComm(int index,QString key);
    void disableUartParseWidgets();
    void enableUartParseWidgets();
    
    //串口操作
    bool uartOpen();
    void uartClose();
    void uartSendData(QString data,bool sendHex, bool sendNewLine);
    void uartSendFile(QString filePath,bool sendHex, bool sendNewLine);
    
    //customPlot操作
    int key;
    void customPlotInit();
    void setupRealtimeDataDemo(QCustomPlot *customPlot);
public slots:
    //串口操作相关槽函数
    void uartOnDataReceived(const QByteArray &);
    void uartOnDataParsed(const UartDataPackage &parsePkg);
    void uartOnDataParsed(const QByteArray &data);
    
    void uartOnBurdRateChanged(int);
    void uartOnSerialNoChanged(int);
    void uartOnSettingMore();
    void uartOnOpen();
    
    void uartOnRcvHexDisplay(int);
    void uartOnRcvSaveToFile(int);
    void uartOnRcvAddTimestamp(int);
    void uartOnRcvClear();
    
    void uartOnSendHex(int);
    void uartOnSendNewLine(int);
    void uartOnSendTimely(int);
    void uartOnSendTimeChanged();
    void sendTimerTimeout();
    void uartOnSend();
    void uartOnSendExpand();
    
    void uartOnParseEnableStateChanged(bool);
    void uartOnParseFixedLengthStateChanged(int);
    void uartOnParseFixedLengthValueChanged(int);
    void uartOnParseHeadTextChanged();
    void uartOnParseTailTextChanged();
    void uartOnParseLengthBytesValueChanged(int);
    void uartOnParseChecksumBytesValueChanged(int);
    void uartOnParseChecksumIndexChanged(int);
    void uartOnParseMoreBtnClicked();
    
    //customplot相关槽函数
    void selectionChanged();
    void mousePress();
    void mouseWheel();
    void moveLegend();
    void graphClicked(QCPAbstractPlottable *plottable, int dataIndex);
    void customPlotReset();
};

#endif // MAINWINDOW_H
