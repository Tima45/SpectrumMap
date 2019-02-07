#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    loadSettings();

    initPlot();
    initSpectrometerLib();
    initTable();
    initScanner();

    updateComCheckBox();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initPlot()
{
    ui->mapPlot->setNoAntialiasingOnDrag(true);
    ui->mapPlot->setInteraction(QCP::iRangeDrag, true);
    ui->mapPlot->setInteraction(QCP::iRangeZoom, true);
    ui->mapPlot->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
    ui->mapPlot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);

    ui->spectrumPlot->setNoAntialiasingOnDrag(true);
    ui->spectrumPlot->setInteraction(QCP::iRangeDrag, true);
    ui->spectrumPlot->setInteraction(QCP::iRangeZoom, true);
    ui->spectrumPlot->axisRect()->setRangeDrag(Qt::Horizontal);
    ui->spectrumPlot->axisRect()->setRangeZoom(Qt::Horizontal);

    spectrumGraph = ui->spectrumPlot->addGraph();

    ui->mapPlot->addLayer("low");
    ui->mapPlot->addLayer("high");
    ui->mapPlot->moveLayer(ui->mapPlot->layer("grid"),ui->mapPlot->layer("high"));


    colorScale = new QCPColorScale(ui->mapPlot);
    colorScale->setType(QCPAxis::atRight);
    colorScale->setGradient(QCPColorGradient(QCPColorGradient::gpHot));
    colorScale->setDataRange(QCPRange(0,1));
    ui->mapPlot->plotLayout()->addElement(0,1,colorScale);


    colorMap = new QCPColorMap(ui->mapPlot->xAxis,ui->mapPlot->yAxis);
    colorMap->setColorScale(colorScale);
    colorMap->setTightBoundary(true);
    colorMap->setLayer("low");
    //colorMap->data()->setSize(50,50);
    colorMap->data()->setRange(QCPRange(-50,50),QCPRange(-50,50));
    colorMap->setInterpolate(false);
    ui->mapPlot->xAxis->setRange(-50,50);
    ui->mapPlot->yAxis->setRange(-50,50);


    spetrometerPos = new QCPItemTracer(ui->mapPlot);
    spetrometerPos->setLayer("high");
    spetrometerPos->setStyle(QCPItemTracer::TracerStyle::tsPlus);
    spetrometerPos->setPen(QPen(QColor(100,255,0)));
    spetrometerPos->setSize(10);

    el = new QCPItemEllipse(ui->mapPlot);
    QPen p1;
    p1.setWidth(2);
    p1.setColor(QColor(Qt::green));
    el->setPen(p1);

    el->topLeft->setCoords(-2,2);
    el->bottomRight->setCoords(2,-2);
    el->setLayer("high");

}

void MainWindow::initSpectrometerLib()
{
    connect(&spectometerLib,SIGNAL(connected()),this,SLOT(switchSpectrometerOnUi()));
    connect(&spectometerLib,SIGNAL(connectionLost()),this,SLOT(switchSpectrometerOffUi()));
    connect(&spectometerLib,SIGNAL(connectionLost()),this,SLOT(stopScanning()));
    connect(&spectometerLib,SIGNAL(newStatus()),this,SLOT(updateSpectometerInfo()));

    connect(&spectometerLib,SIGNAL(newSpectrum()),this,SLOT(drawSpectrum()));
}
void MainWindow::initTable()
{
    connect(&moveTable,SIGNAL(tableConnected()),this,SLOT(setTableIsConnected()));
    connect(&moveTable,SIGNAL(tableDisconnected()),this,SLOT(setTableIsDisconnected()));
    connect(&moveTable,SIGNAL(tableDisconnected()),this,SLOT(stopScanning()));
    connect(&moveTable,SIGNAL(statusUpdated()),this,SLOT(updateStatusInfo()));
    connect(&moveTable,SIGNAL(boundingWarning()),this,SLOT(showWarningBox()));

    serialPortUpdater.setInterval(5000);
    connect(&serialPortUpdater,SIGNAL(timeout()),this,SLOT(updateComCheckBox()));
    //serialPortUpdater.start();


}

void MainWindow::initScanner()
{
    scanner = new SampleScanner();
    scanner->setDevices(&spectometerLib,&moveTable);

    connect(scanner,SIGNAL(scanningStatus(QString)),this,SLOT(updateStatusScanning(QString)));
    connect(this,SIGNAL(startScanning(double,double,double,int)),scanner,SLOT(startScan(double,double,double,int)));
    connect(scanner,SIGNAL(errorWhileScanning(QString)),this,SLOT(showMessageBox(QString)));
    connect(scanner,SIGNAL(scanningFinished()),this,SLOT(thenScanningFinished()));
    connect(scanner,SIGNAL(newResult(QPointF,SpectrumType*)),this,SLOT(updateColorMap(QPointF,SpectrumType*)));
}

bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(result);
    Q_UNUSED(eventType);
    spectometerLib.parseMsg(message);
    return false;
}

void MainWindow::seHvIndicatorValue(int value)
{
    ui->hvValueLabel->setText(QString::number(value)+" V");
    switch(value){
        case 0:{
            ui->hvIndicator->setState(false);
            break;
        }
        case 1450:{
            ui->hvIndicator->setState(true);
            break;
        }
        default:{
            ui->hvIndicator->setLoading();
            break;
        }
    }
}

void MainWindow::setPostEl(double x, double y)
{
    spetrometerPos->position->setCoords(x,y);
    el->topLeft->setCoords(x-2,y+2);
    el->bottomRight->setCoords(x+2,y-2);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);

    spectometerLib.disconnectFromDevice();

    scanner->l.lockForWrite();
    scanner->continueScanning = false;
    scanner->l.unlock();

    serialPortUpdater.stop();

    moveTable.quit();
}



void MainWindow::updateComCheckBox()
{
    if(!moveTable.status.isConnected){
        ui->comPortsBox->clear();
        const auto infos = QSerialPortInfo::availablePorts();
        for (const QSerialPortInfo &info : infos){
            if(!info.isBusy()){
                ui->comPortsBox->addItem(info.portName());
            }
        }
    }
}

void MainWindow::showWarningBox()
{
    QMessageBox::warning(this,"Внимание","Возможен выход за границы, действие отменено");
}

void MainWindow::switchSpectrometerOffUi()
{
    updateComCheckBox();
    ui->spectrometerComPortLabel->setText("");
    ui->connectSpecButton->setText("Подключиться");
    ui->connectionSpectIndicator->setState(false);
    ui->startScanButton->setEnabled(false);
}

void MainWindow::switchSpectrometerOnUi()
{
    updateComCheckBox();
    ui->spectrometerComPortLabel->setText("COM"+QString::number(spectometerLib.deviceInfo.iRadugaPort));
    ui->connectSpecButton->setText("Отключиться");
    ui->connectionSpectIndicator->setState(true);
    if(ui->connectionTableIndicator->isTernedOn()){
        ui->startScanButton->setEnabled(true);
    }
}

void MainWindow::on_setHvButton_clicked()
{
    if(spectometerLib.isConnected()){
        hvOn = !hvOn;
        if(hvOn){
            spectometerLib.setVoltage(1450);
        }else{
            spectometerLib.setVoltage(0);
        }
    }
}

void MainWindow::updateSpectometerInfo()
{
    seHvIndicatorValue(qRound(spectometerLib.extraInfo.sDAC1_CURVAL));
}

void MainWindow::on_connectSpecButton_clicked()
{
    spectometerLib.setMainWindowId(this->windowHandle()->winId());
    if(!spectometerLib.isConnected()){
        ui->connectionSpectIndicator->setLoading();
        int devCount = spectometerLib.getAvailableDevicesCount();
        qDebug() << devCount;
        if(devCount >= 1){
            spectometerLib.connectToDevice(0);
        }else{
            QMessageBox::information(this,"Спектрометр","Не найдено подключенных спектрометров");
            ui->connectionSpectIndicator->setState(false);
        }
    }else{
        spectometerLib.disconnectFromDevice();
    }
}

void MainWindow::on_tableTableButton_clicked()
{
    if(!moveTable.status.isConnected){
        moveTable.setSerialPort(ui->comPortsBox->currentText());
        moveTable.connectToTable();
    }
}

void MainWindow::on_tableFindZeroButton_clicked()
{
    if(moveTable.status.isConnected){
        moveTable.findZero();
    }
}

void MainWindow::on_gotoButton_clicked()
{
    if(moveTable.status.isConnected){ 
        switchButtons(false);
        moveTable.moveTo(ui->XBox->value(),ui->YBox->value());
    }
}

void MainWindow::setTableIsConnected()
{
    ui->connectionTableIndicator->setState(true);
    ui->comPortsBox->setEnabled(false);
    if(ui->connectionSpectIndicator->isTernedOn()){
        ui->startScanButton->setEnabled(true);
    }
}

void MainWindow::setTableIsDisconnected()
{
    if(!moveTable.findingHome){
        isScanning = false;
        ui->connectionTableIndicator->setState(false);
        ui->comPortsBox->setEnabled(true);

        switchButtons(false);
    }
}

void MainWindow::updateStatusInfo()
{
    ui->tableX->setText(QString::number(moveTable.status.X));
    ui->tableY->setText(QString::number(moveTable.status.Y));

    setPostEl(-moveTable.status.X,-moveTable.status.Y);
    ui->mapPlot->replot();
    ui->tableStatusLabel->setText(moveTable.status.idle);

    if(!isScanning){
        switchButtons(moveTable.status.idle == "Idle");
    }


    ui->endPointXIndicator->setState(moveTable.status.PnX);
    ui->endPointYIndicator->setState(moveTable.status.PnY);

    if(moveTable.status.positionMode == TableStatus::Absolute){
        ui->coordsModLabel->setText("Абсолютно");
    }else{
        ui->coordsModLabel->setText("Онтносительно");
    }
    if(!ui->connectionTableIndicator->isTernedOn()){
        ui->connectionTableIndicator->setState(true);
    }
    moveTable.findingHome = false;
}


void MainWindow::on_resetAlarmButton_clicked()
{
    moveTable.resetAlarm();
}

void MainWindow::on_softResetBUtton_clicked()
{
    moveTable.softReset();
}

void MainWindow::on_setZeroButton_clicked()
{
    moveTable.setZero();
}

void MainWindow::on_absoluteRadio_toggled(bool checked)
{
    if(checked){
        moveTable.setAbsolute();
    }
}

void MainWindow::on_relativeRadio_toggled(bool checked)
{
    if(checked){
        moveTable.setRelative();
    }
}

void MainWindow::on_startScanButton_clicked()
{
    if(!isScanning){
        scanner->l.lockForWrite();
        scanner->continueScanning = true;
        scanner->l.unlock();

        isScanning = true;
        ui->scanningIndicator->setState(true);
        ui->startScanButton->setText("Стоп");
        switchButtons(false);
        switchScanButtons(false);

        colorMap->data()->setSize(ui->sampleWidthEdit->value()/ui->strideEdit->value() +1,ui->sampleHeightEdit->value()/ui->strideEdit->value() +1);
        colorMap->data()->setRange(QCPRange(-ui->sampleWidthEdit->value()/2.0,ui->sampleWidthEdit->value()/2.0),QCPRange(-ui->sampleHeightEdit->value()/2.0,ui->sampleHeightEdit->value()/2.0));
        ui->mapPlot->xAxis->setRange(QCPRange(-ui->sampleWidthEdit->value()/2.0,ui->sampleWidthEdit->value()/2.0));
        ui->mapPlot->yAxis->setRange(QCPRange(-ui->sampleHeightEdit->value()/2.0,ui->sampleHeightEdit->value()/2.0));

        ui->mapPlot->replot();


        emit startScanning(ui->sampleWidthEdit->value(),ui->sampleHeightEdit->value(),ui->strideEdit->value(),ui->stoppingTimeEdit->value());
    }else{
        scanner->stopAll();

        stopScanning();
        switchButtons(false);
    }
}

void MainWindow::stopScanning()
{
    isScanning = false;
    ui->scanningIndicator->setState(false);
    ui->startScanButton->setText("Начать измерение");
    switchScanButtons(true);

}

void MainWindow::switchButtons(bool v)
{
    ui->absoluteRadio->setEnabled(v);
    ui->relativeRadio->setEnabled(v);
    ui->gotoButton->setEnabled(v);
    ui->XBox->setEnabled(v);
    ui->YBox->setEnabled(v);
}

void MainWindow::switchScanButtons(bool v)
{
    ui->sampleHeightEdit->setEnabled(v);
    ui->sampleWidthEdit->setEnabled(v);
    ui->strideEdit->setEnabled(v);
    ui->stoppingTimeEdit->setEnabled(v);
}

void MainWindow::showMessageBox(QString text)
{
    QMessageBox::critical(this,"Ошибка",text);
}

void MainWindow::thenScanningFinished()
{
    stopScanning();
}

void MainWindow::updateStatusScanning(QString status)
{
    ui->scanningStatusLabel->setText(status);
    ui->timeStartLabel->setText(scanner->timeStart.toString("hh:mm:ss"));
    ui->timeStopLabel->setText(scanner->timeStop.toString("hh:mm:ss"));
}

void MainWindow::drawSpectrum()
{
    if(x.isEmpty()){
        for(size_t i = 0; i < spectometerLib.spectrum.channelCount; i++){
            x.append(i);
            energy.append(i*C1 + C0);
        }
    }
    y.clear();
    for(size_t i = 0; i < spectometerLib.spectrum.channelCount; i++){
        y.append(spectometerLib.spectrum.chanArray[i]);
    }

    spectrumGraph->data()->clear();

    static bool r = false;
    if(!r){
        r = true;
        ui->spectrumPlot->rescaleAxes();
    }
    ui->spectrumPlot->replot();
}

void MainWindow::updateColorMap(QPointF point, SpectrumType *spectrum)
{
    colorMap->data()->setData(point.x(),point.y(),spectrum->dataCount);
    colorMap->rescaleDataRange();
    ui->mapPlot->replot();
}

void MainWindow::loadSettings()
{
    if(!QFileInfo("Calibration.ini").isFile()){
        QSettings s("Calibration.ini",QSettings::IniFormat);
        s.setValue("C0",5266411);
        s.setValue("C1",121999);
    }
    QSettings s("Calibration.ini",QSettings::IniFormat);
    C0 = s.value("C0").toInt()/1000000.0;
    C1 = s.value("C1").toInt()/1000000.0;

    ui->shownEnergyBox->setMaximum(16384*C0+C1);
    ui->shownEnergyBox->setMinimum(C0+C1);
    ui->shownEnergyBox->setValue((16384/2.0)*C0+C1);
    ui->shownEnergyBox->setSingleStep((16384*C0+C1)*0.01);

    ui->energyIntervalBox->setMaximum((16384*C0+C1)/2.0);
    ui->energyIntervalBox->setMinimum(0);
    ui->energyIntervalBox->setSingleStep((16384*C0+C1)*0.01);
    ui->energyIntervalBox->setValue((16384*C0+C1)*0.02);
}

