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
//    ui->customPlot->xAxis->setRange(-8, 8);
//    ui->customPlot->yAxis->setRange(-5, 5);
//    ui->customPlot->axisRect()->setupFullAxesBox();
    
//    ui->customPlot->xAxis->setLabel("x Axis");
//    ui->customPlot->yAxis->setLabel("y Axis");
    ui->customPlot->legend->setVisible(true);
    QFont legendFont = font();
    legendFont.setPointSize(10);
    ui->customPlot->legend->setFont(legendFont);
    ui->customPlot->legend->setSelectedFont(legendFont);
    ui->customPlot->legend->setSelectableParts(QCPLegend::spItems); // legend box shall not be selectable, only legend items
    
//    addRandomGraph();
//    ui->customPlot->rescaleAxes();
    
    // connect slot that ties some axis selections together (especially opposite axes):
    connect(ui->customPlot, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));
    // connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
    connect(ui->customPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
    connect(ui->customPlot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));
    
    // make bottom and left axes transfer their ranges to top and right axes:
    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->yAxis2, SLOT(setRange(QCPRange)));
        
    // connect slot that shows a message in the status bar when a graph is clicked:
    connect(ui->customPlot, SIGNAL(plottableClick(QCPAbstractPlottable*,int,QMouseEvent*)), this, SLOT(graphClicked(QCPAbstractPlottable*,int)));
    
    // setup policy and connect slot for context menu popup:
    ui->customPlot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->customPlot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));
    
    setupRealtimeDataDemo(ui->customPlot);
}

void MainWindow::setupRealtimeDataDemo(QCustomPlot *customPlot)
{
//  QString demoName = "Real Time Data Demo";
  
  // include this section to fully disable antialiasing for higher performance:
  
//  customPlot->setNotAntialiasedElements(QCP::aeAll);
//  QFont font;
//  font.setStyleStrategy(QFont::NoAntialias);
//  customPlot->xAxis->setTickLabelFont(font);
//  customPlot->yAxis->setTickLabelFont(font);
//  customPlot->legend->setFont(font);
  
  customPlot->addGraph(); // blue line
  customPlot->graph(0)->setPen(QPen(QColor(40, 110, 255)));
  customPlot->addGraph(); // red line
  customPlot->graph(1)->setPen(QPen(QColor(255, 110, 40)));

  QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
  timeTicker->setTimeFormat("%m:%s.%z");
  //timeTicker->setFieldWidth(QCPAxisTickerTime::tuMilliseconds,100);
  timeTicker->setTickStepStrategy( QCPAxisTicker::tssReadability);
  //timeTicker->setTickStepStrategy( QCPAxisTicker::tssMeetTickCount);
  customPlot->xAxis->setTicker(timeTicker);
  customPlot->axisRect()->setupFullAxesBox();
  customPlot->yAxis->setRange(-1.2, 1.2);
  
  // make left and bottom axes transfer their ranges to right and top axes:
  connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
  connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));
  
  // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
  //connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
  //dataTimer.start(0); // Interval 0 means to refresh as fast as possible
}


void MainWindow::realtimeDataSlot()
{
    double displaySec = 8;
  static QTime time(QTime::currentTime());
  // calculate two new data points:
  double key = time.elapsed()/1000.0; // time elapsed since start of demo, in seconds
  static double lastPointKey = 0;
  if (key-lastPointKey > 0.002) // at most add point every 2 ms
  {
    // add data to lines:
    ui->customPlot->graph(0)->addData(key, qSin(key)+qrand()/(double)RAND_MAX*1*qSin(key/0.3843));
    ui->customPlot->graph(1)->addData(key, qCos(key)+qrand()/(double)RAND_MAX*0.5*qSin(key/0.4364));
    // rescale value (vertical) axis to fit the current data:
    //ui->customPlot->graph(0)->rescaleValueAxis();
    //ui->customPlot->graph(1)->rescaleValueAxis(true);
    lastPointKey = key;
  }
  // make key axis range scroll with the data (at a constant range size of 8):
//原始
  //ui->customPlot->xAxis->setRange(key, displaySec, Qt::AlignRight);
//方案1
//  ui->customPlot->xAxis->setRangeLower(key - displaySec);        //右边开始，显示8s，向左平移
//  ui->customPlot->xAxis->setRangeUpper(key); 

//方案2
//  ui->customPlot->xAxis->setRangeLower(0);            //左边开始，大于8s自动延伸
//  ui->customPlot->xAxis->setRangeUpper(key < displaySec ? displaySec : key);
  
//方案3
   ui->customPlot->xAxis->setRangeLower(key < displaySec ? 0 : key-displaySec);  //左边开始，大于8s自动平移
   ui->customPlot->xAxis->setRangeUpper(key < displaySec ? displaySec : key);
  
  ui->customPlot->replot();
  
  // calculate frames per second:
  static double lastFpsKey;
  static int frameCount;
  ++frameCount;
  if (key-lastFpsKey > 2) // average fps over 2 seconds
  {
    ui->statusBar->showMessage(
          QString("%1 FPS, Total Data points: %2")
          .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
          .arg(ui->customPlot->graph(0)->data()->size()+ui->customPlot->graph(1)->data()->size())
          , 0);
    lastFpsKey = key;
    frameCount = 0;
    //this->dataTimer.stop();
    static int totalSec = 0;
    totalSec += 2;
    if(totalSec == 20)
        this->dataTimer.stop();
  }
}

void MainWindow::titleDoubleClick(QMouseEvent* event)
{
  Q_UNUSED(event)
  if (QCPTextElement *title = qobject_cast<QCPTextElement*>(sender()))
  {
    // Set the plot title by double clicking on it
    bool ok;
    QString newTitle = QInputDialog::getText(this, "QCustomPlot example", "New plot title:", QLineEdit::Normal, title->text(), &ok);
    if (ok)
    {
      title->setText(newTitle);
      ui->customPlot->replot();
    }
  }
}

void MainWindow::axisLabelDoubleClick(QCPAxis *axis, QCPAxis::SelectablePart part)
{
  // Set an axis label by double clicking on it
  if (part == QCPAxis::spAxisLabel) // only react when the actual axis label is clicked, not tick label or axis backbone
  {
    bool ok;
    QString newLabel = QInputDialog::getText(this, "QCustomPlot example", "New axis label:", QLineEdit::Normal, axis->label(), &ok);
    if (ok)
    {
      axis->setLabel(newLabel);
      ui->customPlot->replot();
    }
  }
}

void MainWindow::legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item)
{
  // Rename a graph by double clicking on its legend item
  Q_UNUSED(legend)
  if (item) // only react if item was clicked (user could have clicked on border padding of legend where there is no item, then item is 0)
  {
    QCPPlottableLegendItem *plItem = qobject_cast<QCPPlottableLegendItem*>(item);
    bool ok;
    QString newName = QInputDialog::getText(this, "QCustomPlot example", "New graph name:", QLineEdit::Normal, plItem->plottable()->name(), &ok);
    if (ok)
    {
      plItem->plottable()->setName(newName);
      ui->customPlot->replot();
    }
  }
}

void MainWindow::selectionChanged()
{
  /*
   normally, axis base line, axis tick labels and axis labels are selectable separately, but we want
   the user only to be able to select the axis as a whole, so we tie the selected states of the tick labels
   and the axis base line together. However, the axis label shall be selectable individually.
   
   The selection state of the left and right axes shall be synchronized as well as the state of the
   bottom and top axes.
   
   Further, we want to synchronize the selection of the graphs with the selection state of the respective
   legend item belonging to that graph. So the user can select a graph by either clicking on the graph itself
   or on its legend item.
  */
  
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
  // if an axis is selected, only allow the direction of that axis to be dragged
  // if no axis is selected, both directions may be dragged
  
  if (ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->customPlot->axisRect()->setRangeDrag(ui->customPlot->xAxis->orientation());
  else if (ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->customPlot->axisRect()->setRangeDrag(ui->customPlot->yAxis->orientation());
  else
    ui->customPlot->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
}

void MainWindow::mouseWheel()
{
  // if an axis is selected, only allow the direction of that axis to be zoomed
  // if no axis is selected, both directions may be zoomed
  
  if (ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->customPlot->axisRect()->setRangeZoom(ui->customPlot->xAxis->orientation());
  else if (ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->customPlot->axisRect()->setRangeZoom(ui->customPlot->yAxis->orientation());
  else
    ui->customPlot->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
}

void MainWindow::addRandomGraph()
{
  int n = 50; // number of points in graph
  double xScale = (rand()/(double)RAND_MAX + 0.5)*2;
  double yScale = (rand()/(double)RAND_MAX + 0.5)*2;
  double xOffset = (rand()/(double)RAND_MAX - 0.5)*4;
  double yOffset = (rand()/(double)RAND_MAX - 0.5)*10;
  double r1 = (rand()/(double)RAND_MAX - 0.5)*2;
  double r2 = (rand()/(double)RAND_MAX - 0.5)*2;
  double r3 = (rand()/(double)RAND_MAX - 0.5)*2;
  double r4 = (rand()/(double)RAND_MAX - 0.5)*2;
  QVector<double> x(n), y(n);
  for (int i=0; i<n; i++)
  {
    x[i] = (i/(double)n-0.5)*10.0*xScale + xOffset;
    y[i] = (qSin(x[i]*r1*5)*qSin(qCos(x[i]*r2)*r4*3)+r3*qCos(qSin(x[i])*r4*2))*yScale + yOffset;
  }
  
  ui->customPlot->addGraph();
  ui->customPlot->graph()->setName(QString("New graph %1").arg(ui->customPlot->graphCount()-1));
  ui->customPlot->graph()->setData(x, y);
  ui->customPlot->graph()->setLineStyle((QCPGraph::LineStyle)(rand()%5+1));
  if (rand()%100 > 50)
    ui->customPlot->graph()->setScatterStyle(QCPScatterStyle((QCPScatterStyle::ScatterShape)(rand()%14+1)));
  QPen graphPen;
  graphPen.setColor(QColor(rand()%245+10, rand()%245+10, rand()%245+10));
  graphPen.setWidthF(rand()/(double)RAND_MAX*2+1);
  ui->customPlot->graph()->setPen(graphPen);
  ui->customPlot->replot();
}

void MainWindow::removeSelectedGraph()
{
  if (ui->customPlot->selectedGraphs().size() > 0)
  {
    ui->customPlot->removeGraph(ui->customPlot->selectedGraphs().first());
    ui->customPlot->replot();
  }
}

void MainWindow::removeAllGraphs()
{
  ui->customPlot->clearGraphs();
  ui->customPlot->replot();
}

void MainWindow::contextMenuRequest(QPoint pos)
{
  QMenu *menu = new QMenu(this);
  menu->setAttribute(Qt::WA_DeleteOnClose);
  
  if (ui->customPlot->legend->selectTest(pos, false) >= 0) // context menu on legend requested
  {
    menu->addAction("Move to top left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignLeft));
    menu->addAction("Move to top center", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignHCenter));
    menu->addAction("Move to top right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignRight));
    menu->addAction("Move to bottom right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignRight));
    menu->addAction("Move to bottom left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignLeft));
  } else  // general context menu on graphs requested
  {
    menu->addAction("Add random graph", this, SLOT(addRandomGraph()));
    if (ui->customPlot->selectedGraphs().size() > 0)
      menu->addAction("Remove selected graph", this, SLOT(removeSelectedGraph()));
    if (ui->customPlot->graphCount() > 0)
      menu->addAction("Remove all graphs", this, SLOT(removeAllGraphs()));
  }
  
  menu->popup(ui->customPlot->mapToGlobal(pos));
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
  // since we know we only have QCPGraphs in the plot, we can immediately access interface1D()
  // usually it's better to first check whether interface1D() returns non-zero, and only then use it.
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
    
    //bind the uart signals
    connect(ui->btn_uart_setting_open,SIGNAL(clicked()),this,SLOT(uartOnOpen()));
    connect(ui->btn_uart_setting_more,SIGNAL(clicked()),this,SLOT(uartOnSettingMore()));
    connect(ui->btn_uart_send,SIGNAL(clicked()),this,SLOT(uartOnSend()));
    connect(ui->btn_uart_send_choose_file,SIGNAL(clicked()),this,SLOT(uartOnSendChooseFile()));
    connect(ui->btn_uart_rcv_clear,SIGNAL(clicked()),this,SLOT(uartOnRcvClear()));
    connect(ui->btn_uart_rcv_stop,SIGNAL(clicked()),this,SLOT(uartOnRcvStop()));
    connect(ui->checkbox_uart_rcv_add_timestamp,SIGNAL(stateChanged(int)),this,SLOT(uartOnRcvAddTimestamp(int)));
    connect(ui->checkbox_uart_rcv_hex_display,SIGNAL(stateChanged(int)),this,SLOT(uartOnRcvHexDisplay(int)));
    connect(ui->checkbox_uart_rcv_save_to_file,SIGNAL(stateChanged(int)),this,SLOT(uartOnRcvSaveToFile(int)));
    connect(ui->checkbox_uart_send_hex,SIGNAL(stateChanged(int)),this,SLOT(uartOnSendHex(int)));
    connect(ui->checkbox_uart_send_newline,SIGNAL(stateChanged(int)),this,SLOT(uartOnSendNewLine(int)));
    connect(ui->checkbox_uart_send_timely,SIGNAL(stateChanged(int)),this,SLOT(uartOnSendTimely(int)));
    connect(ui->checkbox_uart_setting_dtr,SIGNAL(stateChanged(int)),this,SLOT(uartOnSendDtr(int)));
    connect(ui->checkbox_uart_setting_rts,SIGNAL(stateChanged(int)),this,SLOT(uartOnSendRts(int)));
    connect(ui->box_wav,SIGNAL(toggled(bool)),this,SLOT(wavOnStatusChanged(bool)));
    connect(ui->box_uart_rcv,SIGNAL(toggled(bool)),SLOT(uartRcvOnStatusChanged(bool)));
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
//            ui->btn_uart_send->setEnabled(true);
//            ui->btn_uart_send_choose_file->setEnabled(true);
        }
    }else{
        uartClose();
//        ui->btn_uart_send->setEnabled(false);
//        ui->btn_uart_send_choose_file->setEnabled(false);
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
    int currentTimeout = 500;//ms
    
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
    connect(mySerialPort,SIGNAL(dataReceived(const QByteArray &)),this,SLOT(uartOnDataReceived(const QByteArray &)));
    connect(mySerialPort,SIGNAL(dataParsed(const UartDataPackage &)),this,SLOT(uartOnDataParsed(const UartDataPackage &)));    
    mySerialPort->receiveData();
    
    ui->btn_uart_setting_open->setText(QString::fromLocal8Bit("关闭串口"));
    ui->btn_uart_setting_open->setIcon(QIcon(":/images/uart-state-on.png"));
    return true;
}

void MainWindow::uartOnSend(){
    if(!mySerialPort->isOpen()){
//        QMessageBox::information(this,QString::fromLocal8Bit("WADCS-Monitor"),
//                                 QString::fromLocal8Bit("请先打开串口."));
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

void MainWindow::uartOnSendNewLine(int state){
    //do nothing...
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

void MainWindow::uartOnSendDtr(int state){
    //do nothing...
}
void MainWindow::uartOnSendRts(int state){
    //do nothing...
}

void MainWindow::uartOnRcvClear(){
    ui->textbrowser_uart_receive->clear();
    hasReceivedData.clear();
}
void MainWindow::uartOnRcvStop(){
    mySerialPort->disableReceiving();
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

void MainWindow::uartRcvOnStatusChanged(bool checked){
    mySerialPort->disableReceiving();
}
void MainWindow::uartOnDataReceived(const QByteArray &data){
    QString text;
    hasReceivedData.append(data);
    if(ui->checkbox_uart_rcv_hex_display->checkState()){
        text = StringUtil::convertByteArrayToHexString(data);
    }else{
        text = data;
    }
    appendStringToPlainText(text);
    mySerialPort->parseData(data);   //解析数据
}

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

void MainWindow::wavOnStatusChanged(bool checked){
    //do nothing...
}

void MainWindow::uartOnDataParsed(const UartDataPackage &parsePkg)
{
    double displaySec = 8;
    static QTime time(QTime::currentTime());
    static double lastPointKey = 0;
    // calculate two new data points:
    double key = time.elapsed()/1000.0; // time elapsed since start of demo, in seconds
    
    // add data to lines:
    ui->customPlot->graph(0)->addData(key, parsePkg.data.at(0));
    lastPointKey = key;
    ui->customPlot->xAxis->setRangeLower(key < displaySec ? 0 : key-displaySec);  //左边开始，大于8s自动平移
    ui->customPlot->xAxis->setRangeUpper(key < displaySec ? displaySec : key);
    ui->customPlot->replot();
    
    // calculate frames per second:
    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key-lastFpsKey > 2) // average fps over 2 seconds
    {
        ui->statusBar->showMessage(
                    QString("%1 FPS, Total Data points: %2")
                    .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
                    .arg(ui->customPlot->graph(0)->data()->size()+ui->customPlot->graph(1)->data()->size())
                    , 0);
        lastFpsKey = key;
        frameCount = 0;
        //this->dataTimer.stop();
        static int totalSec = 0;
        totalSec += 2;
    }
}
