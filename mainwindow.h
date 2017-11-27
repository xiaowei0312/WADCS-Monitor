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
    
    //串口操作相关变量
    MySerialPort *mySerialPort;
    QList<UartDataPackage> hasParsedData;
    
    //串口"定时发送"定时器
    QTimer *sendTimer;
private:
    //ui初始化
    void uartSettingsInit();
    void uartEnumation();
    void initWidgets();
    void appendStringToPlainText(QString data);
    QString getComm(int index,QString key);
    
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
    void uartOnOpen();
    void uartOnSettingMore();
    void uartOnSend();
    void uartOnSendChooseFile();
    void uartOnRcvClear();
    void uartOnRcvAddTimestamp(int state);
    void uartOnRcvHexDisplay(int state);
    void uartOnRcvSaveToFile(int state);
    void uartOnSendHex(int state);
    void uartOnSendTimely(int state);
    void uartOnSendTimeChanged();
    void sendTimerTimeout();
    void uartOnDataReceived(const QByteArray &);
    void uartOnDataParsed(const UartDataPackage &parsePkg);
    void uartOnDataParsed(const QByteArray &data);
    
    //customplot相关槽函数
    void selectionChanged();
    void mousePress();
    void mouseWheel();
    void moveLegend();
    void graphClicked(QCPAbstractPlottable *plottable, int dataIndex);
    void customPlotReset();
};

#endif // MAINWINDOW_H
