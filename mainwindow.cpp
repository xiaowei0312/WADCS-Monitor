#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qextserialport.h"
#include "stringutil.h"
#include <QSettings>
#include <QMessageBox>
#include <QScrollBar>
#include <QMetaType>

void MainWindow::customPlotInit()
{
    srand(QDateTime::currentDateTime().toTime_t());
    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                    QCP::iSelectLegend | QCP::iSelectPlottables);
    ui->customPlot->legend->setVisible(true);
    QFont legendFont = font();
    legendFont.setPointSize(10);
    ui->customPlot->legend->setFont(legendFont);
    ui->customPlot->legend->setSelectedFont(legendFont);
    ui->customPlot->legend->setSelectableParts(QCPLegend::spItems); // legend box shall not be selectable, only legend items
    
    connect(ui->customPlot, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));
    connect(ui->customPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
    connect(ui->customPlot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));
    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->yAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlot, SIGNAL(plottableClick(QCPAbstractPlottable*,int,QMouseEvent*)), this, SLOT(graphClicked(QCPAbstractPlottable*,int)));
    
    setupRealtimeDataDemo(ui->customPlot);
}

void MainWindow::setupRealtimeDataDemo(QCustomPlot *customPlot)
{
    customPlot->addGraph(); // blue line
    customPlot->graph(0)->setPen(QPen(QColor(40, 110, 255)));
    customPlot->addGraph(); // red line
    customPlot->graph(1)->setPen(QPen(QColor(255, 110, 40)));
    customPlot->addGraph(); // green line
    customPlot->graph(2)->setPen(QPen(QColor(110, 255, 40)));
    
    customPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc,6));
    customPlot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc,6));
    customPlot->graph(2)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc,6));
    
//    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
//    timeTicker->setTimeFormat("%m:%s.%z");
//    //timeTicker->setFieldWidth(QCPAxisTickerTime::tuMilliseconds,100);
//    timeTicker->setTickStepStrategy( QCPAxisTicker::tssReadability);
//    //timeTicker->setTickStepStrategy( QCPAxisTicker::tssMeetTickCount);
//    customPlot->xAxis->setTicker(timeTicker);
    customPlot->axisRect()->setupFullAxesBox();
    customPlot->yAxis->setRange(0,255);
    customPlot->xAxis->setRange(0,255);
    
    // make left and bottom axes transfer their ranges to right and top axes:
    connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));
}

void MainWindow::selectionChanged()
{
    // make top and bottom axes be selected synchronously, and handle axis and tick labels as one selectable object:
    if (ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
            ui->customPlot->xAxis2->selectedParts().testFlag(QCPAxis::spAxis) || ui->customPlot->xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
    {
        ui->customPlot->xAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
        ui->customPlot->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    }
    // make left and right axes be selected synchronously, and handle axis and tick labels as one selectable object:
    if (ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
            ui->customPlot->yAxis2->selectedParts().testFlag(QCPAxis::spAxis) || ui->customPlot->yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
    {
        ui->customPlot->yAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
        ui->customPlot->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    }
    // synchronize selection of graphs with selection of corresponding legend items:
    for (int i=0; i<ui->customPlot->graphCount(); ++i)
    {
        QCPGraph *graph = ui->customPlot->graph(i);
        QCPPlottableLegendItem *item = ui->customPlot->legend->itemWithPlottable(graph);
        if (item->selected() || graph->selected())
        {
            item->setSelected(true);
            graph->setSelection(QCPDataSelection(graph->data()->dataRange()));
        }
    }
}

void MainWindow::mousePress()
{
    if (ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->customPlot->axisRect()->setRangeDrag(ui->customPlot->xAxis->orientation());
    else if (ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->customPlot->axisRect()->setRangeDrag(ui->customPlot->yAxis->orientation());
    else
        ui->customPlot->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
}

void MainWindow::mouseWheel()
{
    if (ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->customPlot->axisRect()->setRangeZoom(ui->customPlot->xAxis->orientation());
    else if (ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->customPlot->axisRect()->setRangeZoom(ui->customPlot->yAxis->orientation());
    else
        ui->customPlot->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
}

void MainWindow::moveLegend()
{
    if (QAction* contextAction = qobject_cast<QAction*>(sender())) // make sure this slot is really called by a context menu action, so it carries the data we need
    {
        bool ok;
        int dataInt = contextAction->data().toInt(&ok);
        if (ok)
        {
            ui->customPlot->axisRect()->insetLayout()->setInsetAlignment(0, (Qt::Alignment)dataInt);
            ui->customPlot->replot();
        }
    }
}

void MainWindow::graphClicked(QCPAbstractPlottable *plottable, int dataIndex)
{
    double dataValue = plottable->interface1D()->dataMainValue(dataIndex);
    QString message = QString("Clicked on graph '%1' at data point #%2 with value %3.").arg(plottable->name()).arg(dataIndex).arg(dataValue);
    ui->statusBar->showMessage(message, 2500);
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    qRegisterMetaType<UartDataPackage>("UartDataPackage");
    qRegisterMetaType<UartDataPackage>("UartDataPackage&");
    
    ui->setupUi(this);
    
    customPlotInit();
    
    //init the sendTimer
    sendTimer = new QTimer();
    
    //generate the MySerialPort object
    this->mySerialPort = new MySerialPort();
    
    //init uart settings
    uartSettingsInit();
    
    //enum all of uart ports
    uartEnumation();
    
    connect(mySerialPort,SIGNAL(dataReceived(const QByteArray &)),this,SLOT(uartOnDataReceived(const QByteArray &)));
    connect(mySerialPort,SIGNAL(dataParsed(const UartDataPackage &)),this,SLOT(uartOnDataParsed(const UartDataPackage &)));            
    //bind the uart signals
    connect(ui->btn_uart_setting_open,SIGNAL(clicked()),this,SLOT(uartOnOpen()));
    connect(ui->btn_uart_setting_more,SIGNAL(clicked()),this,SLOT(uartOnSettingMore()));
    connect(ui->btn_uart_send,SIGNAL(clicked()),this,SLOT(uartOnSend()));
    connect(ui->btn_uart_send_choose_file,SIGNAL(clicked()),this,SLOT(uartOnSendChooseFile()));
    connect(ui->btn_uart_rcv_clear,SIGNAL(clicked()),this,SLOT(uartOnRcvClear()));
    connect(ui->checkbox_uart_rcv_add_timestamp,SIGNAL(stateChanged(int)),this,SLOT(uartOnRcvAddTimestamp(int)));
    connect(ui->checkbox_uart_rcv_hex_display,SIGNAL(stateChanged(int)),this,SLOT(uartOnRcvHexDisplay(int)));
    connect(ui->checkbox_uart_rcv_save_to_file,SIGNAL(stateChanged(int)),this,SLOT(uartOnRcvSaveToFile(int)));
    connect(ui->checkbox_uart_send_hex,SIGNAL(stateChanged(int)),this,SLOT(uartOnSendHex(int)));
    connect(ui->checkbox_uart_send_timely,SIGNAL(stateChanged(int)),this,SLOT(uartOnSendTimely(int)));
    connect(ui->edit_uart_send_timely,SIGNAL(editingFinished()),this,SLOT(uartOnSendTimeChanged()));
    connect(sendTimer,SIGNAL(timeout()),this,SLOT(sendTimerTimeout()));
    
    //init the widgets
    initWidgets();
}

MainWindow::~MainWindow()
{
    delete ui;
    this->mySerialPort->close();
    delete this->mySerialPort;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::initWidgets()
{
    //    ui->btn_uart_send->setEnabled(false);
    //    ui->btn_uart_send_choose_file->setEnabled(false);
    ui->edit_uart_send_timely->setValidator(
                new QRegExpValidator(
                    QRegExp("^([1-9]\\d{0,15}|0)$"),this));
    ui->edit_uart_send_timely->setText(QString::number(1000));
}

void MainWindow::uartSettingsInit()
{
#if 0
    enum BaudRateType 
    {
        BAUD50,                //POSIX ONLY
        BAUD75,                //POSIX ONLY
        BAUD110,
        BAUD134,               //POSIX ONLY
        BAUD150,               //POSIX ONLY
        BAUD200,               //POSIX ONLY
        BAUD300,
        BAUD600,
        BAUD1200,
        BAUD1800,              //POSIX ONLY
        BAUD2400,
        BAUD4800,
        BAUD9600,
        BAUD14400,             //WINDOWS ONLY
        BAUD19200,
        BAUD38400,
        BAUD56000,             //WINDOWS ONLY
        BAUD57600,
        BAUD76800,             //POSIX ONLY
        BAUD115200,
        BAUD128000,            //WINDOWS ONLY
        BAUD256000             //WINDOWS ONLY
    };
#endif
    //波特率列表必须与qextserialbase.h中枚举类型对应
    buadrateList << "50" << "75" << "110" << "134" << "150"
                 << "200" << "300" << "600" << "1200" << "1800" << "2400"
                 << "4800" << "9600" << "14400" << "19200" << "38400" << "56000"
                 << "57600" << "76800" << "115200" << "128000" << "256000";  
#if 0
    enum DataBitsType 
    {
        DATA_5,
        DATA_6,
        DATA_7,
        DATA_8
    };
#endif
    databitList << "5" << "6" << "7" << "8";
    
#if 0
    enum StopBitsType 
    {
        STOP_1,
        STOP_1_5,               //WINDOWS ONLY
        STOP_2
    };
#endif
    stopbitList << "1" << "1.5" << "2";
    
#if 0
    enum ParityType 
    {
        PAR_NONE,
        PAR_ODD,
        PAR_EVEN,
        PAR_MARK,               //WINDOWS ONLY
        PAR_SPACE
    };
    
    enum FlowType 
    {
        FLOW_OFF,
        FLOW_HARDWARE,
        FLOW_XONXOFF
    };
#endif
    ui->combo_uart_setting_burdrate->addItems(buadrateList);
    ui->combo_uart_setting_burdrate->setCurrentIndex(12);
    ui->combo_uart_setting_databit->addItems(databitList);
    ui->combo_uart_setting_databit->setCurrentIndex(3);
    ui->combo_uart_setting_stopbit->addItems(stopbitList);
    ui->combo_uart_setting_stopbit->setCurrentIndex(0);
}

void MainWindow::uartEnumation()
{
    QString  path = "HKEY_LOCAL_MACHINE\\HARDWARE\\DEVICEMAP\\SERIALCOMM";     
    QSettings *settings = new  QSettings(path,QSettings::NativeFormat);      
    QStringList keyList = settings->allKeys();
    for (int i = 0; i < keyList.size(); i++){
        QString portName =  getComm(i,"value");
        if(portName.isEmpty()){
            QMessageBox::critical(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("无法打开注册表，请确认是否有相关权限."));
            break;
        }
        uartPortList << portName;
    }
    ui->combo_uart_setting_serail_no->addItems(uartPortList);
}

QString MainWindow::getComm(int index,QString keyorvalue)
{
    QString commresult = "";
    QString strkey = "HARDWARE\\DEVICEMAP\\SERIALCOMM";//子键路径     
    int a = strkey.toWCharArray(subkey);
    subkey[a] = '\0';
    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,subkey,0,KEY_READ|KEY_QUERY_VALUE,&hKey)!=0){
        QString error = "Cannot open regedit!";
        qDebug() << error;
        return "";
    }
    QString keymessage = "";//键名     
    QString message = "";      
    QString valuemessage = "";//键值       
    indexnum = index;//要读取键值的索引号         
    keysize = sizeof(keyname);      
    valuesize = sizeof(keyvalue);
    if(::RegEnumValue(hKey,indexnum,keyname,&keysize,0,&type,(BYTE*)keyvalue,&valuesize)== 0){//读取键名          
        for(unsigned int i=0; i<keysize; i++){
            message=QString::fromStdWString(keyname);             
            keymessage.append(message);         
        }
        //读取键值          
        for(unsigned int j=0;j<valuesize;j++){              
            if(keyvalue[j]!=0x00){                  
                valuemessage.append(keyvalue[j]);             
            }
        }         
        if(keyorvalue == "key"){              
            commresult = keymessage;         
        }          
        if(keyorvalue == "value"){              
            commresult = valuemessage;         
        }     
    }
    else{          
        commresult="nokey";
    }
    ::RegCloseKey(hKey);//关闭注册表     
    return commresult;  
}

void MainWindow::uartOnOpen(){
    QString text = ui->btn_uart_setting_open->text();
    if(text == QString::fromLocal8Bit("打开串口")){
        if(uartOpen()){
        }
    }else{
        uartClose();
    }
}

void MainWindow::uartClose(){
    mySerialPort->close();
    ui->btn_uart_setting_open->setText("打开串口");
    ui->btn_uart_setting_open->setIcon(QIcon(":/images/uart-state-off.png"));
}
bool MainWindow::uartOpen(){
    int currentPort = ui->combo_uart_setting_serail_no->currentIndex();
    int currentBardrate = ui->combo_uart_setting_burdrate->currentIndex();
    int currentDataBit = ui->combo_uart_setting_databit->currentIndex();
    int currentStopBit = ui->combo_uart_setting_stopbit->currentIndex();
    int currentParity = 0;
    int currentFlowControl = 0;
    int currentTimeout = 10;//ms
    
    bool success = mySerialPort->open(uartPortList[currentPort],
                                      (BaudRateType)currentBardrate,(DataBitsType)currentDataBit,
                                      (ParityType)currentParity,(StopBitsType)currentStopBit,
                                      (FlowType)currentFlowControl,0,currentTimeout);
    if(!success){
        QMessageBox::critical(this,QString::fromLocal8Bit("WADCS-Monitor"),
                              QString::fromLocal8Bit("打开串口失败，串口被其他程序占用或者没有相关权限."));
        return false;
    }
    mySerialPort->enableSending();
    mySerialPort->enableReceiving();
    mySerialPort->enableParsing();
    mySerialPort->receiveData();
    
    ui->btn_uart_setting_open->setText(QString::fromLocal8Bit("关闭串口"));
    ui->btn_uart_setting_open->setIcon(QIcon(":/images/uart-state-on.png"));
    return true;
}

void MainWindow::uartOnSend(){
    if(!mySerialPort->isOpen()){
        QMessageBox::information(this,QString::fromLocal8Bit("WADCS-Monitor"),
                                 QString::fromLocal8Bit("请先打开串口."));
        return;
    }
    
    QString data = ui->lineedit_uart_send->text();
    bool sendHex = ui->checkbox_uart_send_hex->checkState();
    bool sendNewLine = ui->checkbox_uart_send_newline->checkState();
    
    if(data.startsWith("[FILE]") && data.endsWith("[FILE]")){
        QString filePath = data.right(strlen("[FILE]"));
        filePath = filePath.left(strlen("[FILE]"));
        qDebug() << filePath;
        uartSendFile(filePath,sendHex,sendNewLine);
    }else{
        uartSendData(data,sendHex,sendNewLine);   
    }
}

void MainWindow::uartSendData(QString data,bool sendAsHex, bool sendNewLine){
    QByteArray unSendData;
    if(!sendAsHex){
        if(sendNewLine)
            data += "\r\n";
        unSendData = data.toLatin1();
    }else{
        if(data.isEmpty())
            return;
        if(!StringUtil::validHexStr(data)){
            QMessageBox::information(this,QString::fromLocal8Bit("WADCS-Monitor"),
                                     QString::fromLocal8Bit("请键入有效Hex字符组合(\"0-9\",\"A-F\",\"a-f\")!\n每两个字符之间空一个空格"));
            return;
        }
        //unSendData = QByteArray::fromHex(data.remove(QRegExp("\\s")).toLatin1());
        unSendData = StringUtil::convertHexStringToByteArray(data);
    }
    mySerialPort->sendData(unSendData);
}

void MainWindow::uartSendFile(QString filePath,bool sendHex, bool sendNewLine){
    
}

void MainWindow::uartOnSendChooseFile(){
    
}

void MainWindow::uartOnSendHex(int state){
    if(state)
        ui->checkbox_uart_send_newline->setEnabled(false);
    else
        ui->checkbox_uart_send_newline->setEnabled(true);
}

void MainWindow::uartOnSendTimely(int state){
    if(state){
        int milltimes = ui->edit_uart_send_timely->text().toInt();
        if(milltimes <= 0)
            return;
        sendTimer->start(milltimes);
    }else{
        sendTimer->stop();
    }
}

void MainWindow::uartOnSendTimeChanged()
{
    QString text =  ui->edit_uart_send_timely->text();
    if(text.isEmpty())
        return;
    int sendTimeInterval = text.toInt();
    if(sendTimeInterval>0){
        sendTimer->setInterval(sendTimeInterval);
    }
}

void MainWindow::sendTimerTimeout(){
    this->uartOnSend();
}

void MainWindow::uartOnSettingMore(){
    
}

void MainWindow::uartOnRcvClear(){
    ui->textbrowser_uart_receive->clear();
    ui->customPlot->graph(0)->setData(QVector<double>(),QVector<double>());
    ui->customPlot->graph(1)->setData(QVector<double>(),QVector<double>());
    ui->customPlot->graph(2)->setData(QVector<double>(),QVector<double>());
    ui->customPlot->replot();
    
    hasReceivedData.clear();
}
void MainWindow::uartOnRcvAddTimestamp(int state){}
void MainWindow::uartOnRcvHexDisplay(int state){
    QString data;
    if(state){  //checked
        data = StringUtil::convertByteArrayToHexString(hasReceivedData);
    }else{
        data = hasReceivedData;
    }
    ui->textbrowser_uart_receive->clear();
    appendStringToPlainText(data);
}

void MainWindow::uartOnRcvSaveToFile(int state){}

void MainWindow::appendStringToPlainText(QString text)
{
    //追加文本（ui.taskStatusTextEdit是一个QPlainTextEdit对象）
    QTextCursor workCursor = ui->textbrowser_uart_receive->textCursor();
    workCursor.movePosition(QTextCursor::End);
    workCursor.insertText(text);
    //workCursor.insertBlock();
    
    //移动滚动条到底部
    QScrollBar *scrollbar = ui->textbrowser_uart_receive->verticalScrollBar();
    if (scrollbar){
        scrollbar->setSliderPosition(scrollbar->maximum());
    }
}

void MainWindow::uartOnDataReceived(const QByteArray &data){
    QString text;
    hasReceivedData.append(data);
    unParsedData.append(data);
    if(ui->checkbox_uart_rcv_hex_display->checkState()){
        text = StringUtil::convertByteArrayToHexString(data);
    }else{
        text = data;
    }
    appendStringToPlainText(text);
    mySerialPort->parseData(data);   //解析数据
    //mySerialPort->parseData(unParsedData);  //解析数据
}

//void MainWindow::uartOnDataParsed(const UartDataPackage &parsePkg)
//{
//    if(!parsePkg.isValid){   //无效包，丢弃
//        qDebug("%s","无效包，丢弃");
//        return;
//    }
////    QTime now = QTime::currentTime();
////    qDebug("%d.%d\thead: %02x%02x   length: %02x    data: %02x%02x%02x     valid: %s",
////           now.second(),now.msec(),
////           parsePkg.head[0] & 0xFF,parsePkg.head[1] & 0xFF,
////            parsePkg.length & 0xFF,
////            parsePkg.data[0] & 0xFF,parsePkg.data[1] & 0xFF,parsePkg.data[2] & 0xFF,
////            parsePkg.isValid ? "有效" : "无效");
    
//    double displaySec = 8;
//    static QTime time(QTime::currentTime());
//    // calculate two new data points:
//    double key = time.elapsed()/1000.0; // time elapsed since start of demo, in seconds
    
//    // add data to lines:
//    for(int i=0;i<parsePkg.data.length();i++){
//        ui->customPlot->graph(0)->addData(key, (unsigned int)(parsePkg.data[i] & 0xFF));
//    }
////    ui->customPlot->graph(1)->addData(key, (unsigned int)(parsePkg.data[1] & 0xFF));
////    ui->customPlot->graph(2)->addData(key, (unsigned int)(parsePkg.data[2] & 0xFF));
//    ui->customPlot->xAxis->setRangeLower(key < displaySec ? 0 : key-displaySec);  //左边开始，大于8s自动平移
//    ui->customPlot->xAxis->setRangeUpper(key < displaySec ? displaySec : key);
//    ui->customPlot->replot();
    
//    // calculate frames per second:
//    static double lastFpsKey;
//    static int frameCount;
//    ++frameCount;
//    if (key-lastFpsKey > 2) // average fps over 2 seconds
//    {
//        ui->statusBar->showMessage(
//                    QString("%1 FPS, Total Data points: %2")
//                    .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
//                    .arg(ui->customPlot->graph(0)->data()->size()+ui->customPlot->graph(1)->data()->size())
//                    , 0);
//        lastFpsKey = key;
//        frameCount = 0;
//        //this->dataTimer.stop();
//        static int totalSec = 0;
//        totalSec += 2;
//    }
//}


void MainWindow::uartOnDataParsed(const UartDataPackage &parsePkg)
{
    if(!parsePkg.isValid){   //无效包，丢弃
        qDebug("%s","无效包，丢弃");
        return;
    }
    
    double displaySec = 256;
    static QTime time(QTime::currentTime());
    // calculate two new data points:
    double key = time.elapsed()/1000.0; // time elapsed since start of demo, in seconds
    
    // add data to lines:
    for(int i=0;i<parsePkg.data.length();i++){
        ui->customPlot->graph(0)->addData(i, (unsigned int)(parsePkg.data[i] & 0xFF));
    }
//    ui->customPlot->graph(1)->addData(key, (unsigned int)(parsePkg.data[1] & 0xFF));
//    ui->customPlot->graph(2)->addData(key, (unsigned int)(parsePkg.data[2] & 0xFF));
//    ui->customPlot->xAxis->setRangeLower(key < displaySec ? 0 : key-displaySec);  //左边开始，大于8s自动平移
//    ui->customPlot->xAxis->setRangeUpper(key < displaySec ? displaySec : key);
    ui->customPlot->replot();
}
