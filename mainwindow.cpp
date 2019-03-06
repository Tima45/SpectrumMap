#include "mainwindow.h"
#include "ui_mainwindow.h"

void saveDWORDToStrem(QDataStream &str,DWORD &w){
    char *bytes = (char *)&w;
    str.writeRawData(bytes,sizeof(DWORD));
}

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


    ui->spectrumPlot->setNoAntialiasingOnDrag(true);
    ui->spectrumPlot->setInteraction(QCP::iRangeDrag, true);
    ui->spectrumPlot->setInteraction(QCP::iRangeZoom, true);
    ui->spectrumPlot->axisRect()->setRangeDrag(Qt::Horizontal);
    ui->spectrumPlot->axisRect()->setRangeZoom(Qt::Horizontal);

    energyLine = new QCPItemLine(ui->spectrumPlot);
    energyLineLeft = new QCPItemLine(ui->spectrumPlot);
    energyLineRgiht = new QCPItemLine(ui->spectrumPlot);

    setLineKey(energyLine,ui->shownEnergyBox->value());
    setLineKey(energyLineLeft,ui->shownEnergyBox->value()-ui->energyIntervalBox->value());
    setLineKey(energyLineRgiht,ui->shownEnergyBox->value()+ui->energyIntervalBox->value());




    connect(ui->spectrumPlot,SIGNAL(mouseDoubleClick(QMouseEvent*)),this,SLOT(handleClick(QMouseEvent*)));

    spectrumGraph = ui->spectrumPlot->addGraph();
    spectrumGraph->addData(toEnergy(0),1);
    spectrumGraph->addData(toEnergy(MAXCHANNEL),1);

    ui->spectrumPlot->rescaleAxes();
    ui->spectrumPlot->replot();



    ui->mapPlot->addLayer("low");
    ui->mapPlot->addLayer("high");
    ui->mapPlot->moveLayer(ui->mapPlot->layer("grid"),ui->mapPlot->layer("high"));


    colorScale = new QCPColorScale(ui->mapPlot);
    colorScale->setType(QCPAxis::atRight);
    colorScale->setGradient(QCPColorGradient(QCPColorGradient::gpThermal));
    colorScale->setDataRange(QCPRange(0,1));
    ui->mapPlot->plotLayout()->addElement(0,1,colorScale);


    colorMap = new QCPColorMap(ui->mapPlot->xAxis,ui->mapPlot->yAxis);
    colorMap->setColorScale(colorScale);
    colorMap->setTightBoundary(true);
    colorMap->setLayer("low");
    //colorMap->data()->setSize(50,50);
    colorMap->data()->setRange(QCPRange(-60,60),QCPRange(-60,60));
    colorMap->setInterpolate(false);
    ui->mapPlot->xAxis->setRange(-60,60);
    ui->mapPlot->yAxis->setRange(-60,60);

    ui->mapPlot->setInteraction(QCP::iRangeDrag, true);
    ui->mapPlot->setInteraction(QCP::iRangeZoom, true);
    ui->mapPlot->xAxis->ticker()->setTickCount(10);
    ui->mapPlot->yAxis->ticker()->setTickCount(10);
    ui->mapPlot->axisRect()->setRangeDrag(0);
    ui->mapPlot->axisRect()->setRangeZoom(0);


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
    //connect(&moveTable,SIGNAL(tableDisconnected()),this,SLOT(setTableIsDisconnected()));
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

void MainWindow::setLineKey(QCPItemLine *line, double key)
{
    if(line){
        line->start->setCoords(key,0);
        line->end->setCoords(key,MAXCHANNEL);
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
        scanner->continueScanning = true;
        isScanning = true;
        ui->scanningIndicator->setState(true);
        ui->startScanButton->setText("Стоп");
        switchButtons(false);
        switchScanButtons(false);

        colorMap->data()->clear();
        colorMap->data()->setSize(ui->sampleWidthEdit->value()/ui->strideEdit->value() +1,ui->sampleHeightEdit->value()/ui->strideEdit->value() +1);
        colorMap->data()->setRange(QCPRange(-ui->sampleWidthEdit->value()/2.0,ui->sampleWidthEdit->value()/2.0),QCPRange(-ui->sampleHeightEdit->value()/2.0,ui->sampleHeightEdit->value()/2.0));
        ui->mapPlot->xAxis->setRange(QCPRange(-ui->sampleWidthEdit->value()/2.0,ui->sampleWidthEdit->value()/2.0));
        ui->mapPlot->yAxis->setRange(QCPRange(-ui->sampleHeightEdit->value()/2.0,ui->sampleHeightEdit->value()/2.0));


        ui->mapPlot->xAxis->ticker()->setTickCount(ui->sampleWidthEdit->value()/ui->strideEdit->value());
        ui->mapPlot->yAxis->ticker()->setTickCount(ui->sampleWidthEdit->value()/ui->strideEdit->value());


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
    spectrumGraph->setData(x,y);

    static bool r = false;
    if(!r){
        r = true;
        ui->spectrumPlot->rescaleAxes();
    }
    ui->spectrumPlot->replot();
}

void MainWindow::updateColorMap(QPointF point, SpectrumType *spectrum)
{
    colorMap->data()->setData(point.x(),point.y(),countSummOnInterval(spectrum));
    if(autoRescaleData){
        colorMap->rescaleDataRange();
    }
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

    ui->shownEnergyBox->setMaximum(toEnergy(MAXCHANNEL));
    ui->shownEnergyBox->setMinimum(toEnergy(1));
    ui->shownEnergyBox->setValue(toEnergy(MAXCHANNEL/3.0));
    ui->shownEnergyBox->setSingleStep(toEnergy(MAXCHANNEL)*0.001);

    ui->energyIntervalBox->setMaximum(toEnergy(MAXCHANNEL)/2.0);
    ui->energyIntervalBox->setMinimum(1);
    ui->energyIntervalBox->setSingleStep(toEnergy(MAXCHANNEL)*0.001);
    ui->energyIntervalBox->setValue(toEnergy(MAXCHANNEL)*0.002);
}

int MainWindow::toChanal(double value)
{
    return qRound((value-C0)/C1);
}


double MainWindow::toEnergy(int c)
{
    return 1.0*c*C1+C0;
}

double MainWindow::countSummOnInterval(SpectrumType *spectrum)
{
    double energy = ui->shownEnergyBox->value();
    double interval = ui->energyIntervalBox->value();
    int chanal = toChanal(energy);
    int intervalInChanals = toChanal(interval);

    double summ = 0;
    for(int i = chanal-intervalInChanals; i < chanal+intervalInChanals; ++i){
        if(i >= 0 && i < MAXCHANNEL){
            summ += spectrum->chanArray[i];
        }
    }
    return summ;
}



void MainWindow::on_savePicButton_clicked()
{
    if(!scanner->continueScanning){
        //if(!scanner->resultMap.isEmpty()){
            QString filePath = QFileDialog::getSaveFileName(this,"Сохранить карту","","2d spectrum maps(*.smap)");
            if(!filePath.isEmpty()){


                QFile f(filePath);
                f.open(QIODevice::WriteOnly);
                QDataStream str(&f);
                str << scanner->width;
                str << scanner->height;
                str << scanner->stride;

                str << scanner->timeStart;
                str << scanner->timeStop;

                str << scanner->resultMap.count();

                for(int i = 0; i < scanner->resultMap.count(); i++){

                    qDebug() << scanner->resultMap.at(i).first.x() << scanner->resultMap.at(i).first.y();

                  str << scanner->resultMap.at(i).first.x();
                  str << scanner->resultMap.at(i).first.y();

                  str << scanner->resultMap.at(i).second->channelCount;
                  str << scanner->resultMap.at(i).second->dataCount;
                  str << scanner->resultMap.at(i).second->dataIsNew;

                  saveDWORDToStrem(str,scanner->resultMap.at(i).second->dataTime);
                  saveDWORDToStrem(str,scanner->resultMap.at(i).second->liveTime);
                  saveDWORDToStrem(str,scanner->resultMap.at(i).second->workTime);
                  qDebug() << "saved.";
                  for(int j = 0; j < MAXCHANNEL; j++){
                    saveDWORDToStrem(str,scanner->resultMap.at(i).second->chanArray[j]);
                  }
                }
                f.close();
            }
        }
    //}
}


void MainWindow::on_openPicButton_clicked()
{
    if(!scanner->continueScanning){
        QString filePath = QFileDialog::getOpenFileName(this,"Открыть карту","","2d spectrum maps(*.smap)");
        if(!filePath.isEmpty()){

            scanner->clearSpectrumMap();

            QFile f(filePath);
            f.open(QIODevice::ReadOnly);
            QDataStream str(&f);

            str >> scanner->width;
            str >> scanner->height;
            str >> scanner->stride;

            qDebug() << scanner->width;
            qDebug() << scanner->height;
            qDebug() << scanner->stride;

            str >> scanner->timeStart;
            str >> scanner->timeStop;


            qDebug() << scanner->timeStart.toString("hh:mm:ss");
            qDebug() << scanner->timeStop.toString("hh:mm:ss");

            int resultMapCount = 0;
            str >> resultMapCount;

            qDebug() << resultMapCount;
            for(int i = 0; i < resultMapCount; i++){

              QPointF p;
              qreal x,y;
              str >> x;
              str >> y;
              p.setX(x);
              p.setX(y);



              qDebug() << p.x() << p.y();

              SpectrumType *newSpectrum = new SpectrumType();

              str >> newSpectrum->channelCount;
              str >> newSpectrum->dataCount;



              str >> newSpectrum->dataIsNew;


              DWORD temp = 0;
              char* chartmp = (char*)&temp;
              uint size = sizeof(DWORD);

              str.readBytes(chartmp,size);
              newSpectrum->dataTime = temp;
              str.readBytes(chartmp,size);
              newSpectrum->liveTime = temp;
              str.readBytes(chartmp,size);
              newSpectrum->workTime = temp;
              for(int j = 0; j < 16384; j++){
                  str.readBytes(chartmp,size);
                  newSpectrum->chanArray[j] = temp;
              }
              QPair<QPointF,SpectrumType*> pair;
              pair.first = p;
              pair.second = newSpectrum;
              scanner->resultMap.append(pair);
            }
            f.close();
            showLoadedOnMap();
        }
    }
}

void MainWindow::showLoadedOnMap()
{
    ui->sampleWidthEdit->setValue(scanner->width);
    ui->sampleHeightEdit->setValue(scanner->height);
    ui->strideEdit->setValue(scanner->stride);

    ui->timeStartLabel->setText(scanner->timeStart.toString("hh:mm:ss"));
    ui->timeStopLabel->setText(scanner->timeStop.toString("hh:mm:ss"));

    colorMap->data()->clear();
    colorMap->data()->setSize(ui->sampleWidthEdit->value()/ui->strideEdit->value() +1,ui->sampleHeightEdit->value()/ui->strideEdit->value() +1);
    colorMap->data()->setRange(QCPRange(-ui->sampleWidthEdit->value()/2.0,ui->sampleWidthEdit->value()/2.0),QCPRange(-ui->sampleHeightEdit->value()/2.0,ui->sampleHeightEdit->value()/2.0));
    ui->mapPlot->xAxis->setRange(QCPRange(-ui->sampleWidthEdit->value()/2.0,ui->sampleWidthEdit->value()/2.0));
    ui->mapPlot->yAxis->setRange(QCPRange(-ui->sampleHeightEdit->value()/2.0,ui->sampleHeightEdit->value()/2.0));


    ui->mapPlot->xAxis->ticker()->setTickCount(ui->sampleWidthEdit->value()/ui->strideEdit->value());
    ui->mapPlot->yAxis->ticker()->setTickCount(ui->sampleWidthEdit->value()/ui->strideEdit->value());


    for(int i = 0; i < scanner->resultMap.count(); i++){
        colorMap->data()->setData(scanner->resultMap.at(i).first.x(),scanner->resultMap.at(i).first.y(),countSummOnInterval(scanner->resultMap.at(i).second));

    }
    colorMap->rescaleDataRange();
    ui->mapPlot->replot();

}


void MainWindow::on_groupBox_4_clicked(bool checked)
{
    toggleVisibleOnLayout(ui->groupBox_4->layout(),checked);
}

void MainWindow::toggleVisibleOnLayout(QLayout *lay,bool v)
{
    if(lay){
        for(int i = 0; i < lay->count(); ++i){
            QWidget *w = lay->itemAt(i)->widget();
            QLayout *l = lay->itemAt(i)->layout();
            if(w){
                w->setVisible(v);
            }
            if(l){
                toggleVisibleOnLayout(l,v);
            }
        }
    }
}

void MainWindow::on_groupBox_3_clicked(bool checked)
{
    toggleVisibleOnLayout(ui->groupBox_3->layout(),checked);
}

void MainWindow::on_groupBox_2_clicked(bool checked)
{
    toggleVisibleOnLayout(ui->groupBox_2->layout(),checked);
}

void MainWindow::on_groupBox_clicked(bool checked)
{
    toggleVisibleOnLayout(ui->groupBox->layout(),checked);
}
void MainWindow::handleClick(QMouseEvent *)
{
    freeIntaraction = !freeIntaraction;
    if(freeIntaraction){
        ui->spectrumPlot->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
        ui->spectrumPlot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
    }else{
        ui->spectrumPlot->axisRect()->setRangeDrag(Qt::Horizontal);
        ui->spectrumPlot->axisRect()->setRangeZoom(Qt::Horizontal);
    }
}

void MainWindow::updateColorMap()
{
    if(colorMap){
        for(int i = 0; i < scanner->resultMap.count(); i++){
            QPointF p = scanner->resultMap.at(i).first;
            SpectrumType *spectrum = scanner->resultMap.at(i).second;
            colorMap->data()->setData(p.x(),p.y(),countSummOnInterval(spectrum));
        }
        if(autoRescaleData){
            colorMap->rescaleDataRange();
        }
        ui->mapPlot->replot();
    }
}

void MainWindow::on_shownEnergyBox_valueChanged(double arg1)
{
    setLineKey(energyLine,arg1);
    setLineKey(energyLineLeft,arg1-ui->energyIntervalBox->value());
    setLineKey(energyLineRgiht,arg1+ui->energyIntervalBox->value());

    updateColorMap();
    ui->spectrumPlot->replot();
}

void MainWindow::on_energyIntervalBox_valueChanged(double arg1)
{
    setLineKey(energyLineLeft,ui->shownEnergyBox->value()-arg1);
    setLineKey(energyLineRgiht,ui->shownEnergyBox->value()+arg1);
    updateColorMap();
    ui->spectrumPlot->replot();
}
