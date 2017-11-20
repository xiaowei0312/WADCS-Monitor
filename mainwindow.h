#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <windows.h>
#include <QInputDialog>
#include "myserialport.h"
#include "qcustomplot.h"

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
    
    wchar_t subkey[80];          
    wchar_t keyname[256]; //键名数组        
    char keyvalue[256];  //键值数组         
    int indexnum;          
    DWORD keysize,type,valuesize;         
    HKEY hKey; 
    
    QList<QString> uartPortList;
    QList<QString> buadrateList;
    QList<QString> databitList;
    QList<QString> stopbitList;
    MySerialPort *mySerialPort;
    QByteArray hasReceivedData;
    QTimer *sendTimer;
private:
    QTimer dataTimer;
    void customPlotInit();
    void setupRealtimeDataDemo(QCustomPlot *customPlot);
private slots:
    void realtimeDataSlot();
    void titleDoubleClick(QMouseEvent *event);
    void axisLabelDoubleClick(QCPAxis* axis, QCPAxis::SelectablePart part);
    void legendDoubleClick(QCPLegend* legend, QCPAbstractLegendItem* item);
    void selectionChanged();
    void mousePress();
    void mouseWheel();
    void addRandomGraph();
    void removeSelectedGraph();
    void removeAllGraphs();
    void contextMenuRequest(QPoint pos);
    void moveLegend();
    void graphClicked(QCPAbstractPlottable *plottable, int dataIndex);
private:
    void uartSettingsInit();
    void uartEnumation();
    void initWidgets();
    void appendStringToPlainText(QString data);
    QString getComm(int index,QString key);
    
    bool uartOpen();
    void uartClose();
    void uartSendData(QString data,bool sendHex, bool sendNewLine);
    void uartSendFile(QString filePath,bool sendHex, bool sendNewLine);
public slots:
    void uartOnOpen();
    void uartOnSettingMore();
    void uartOnSend();
    void uartOnSendChooseFile();
    void uartOnRcvClear();
    void uartOnRcvStop();
    void uartOnRcvAddTimestamp(int state);
    void uartOnRcvHexDisplay(int state);
    void uartOnRcvSaveToFile(int state);
    void uartOnSendHex(int state);
    void uartOnSendNewLine(int state);
    void uartOnSendTimely(int state);
    void uartOnSendDtr(int state);
    void uartOnSendRts(int state);
    void uartRcvOnStatusChanged(bool checked);
    void wavOnStatusChanged(bool checked);
    void uartOnSendTimeChanged();
    void sendTimerTimeout();
    void uartOnDataReceived(const QByteArray &);
};

#endif // MAINWINDOW_H
