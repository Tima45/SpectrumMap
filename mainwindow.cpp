#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initPlot();
    initSpectrometerLib();
    initTable();

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
}

void MainWindow::initSpectrometerLib()
{
    connect(&spectometerLib,SIGNAL(connected()),this,SLOT(switchSpectrometerOnUi()));
    connect(&spectometerLib,SIGNAL(connectionLost()),this,SLOT(switchSpectrometerOffUi()));
    connect(&spectometerLib,SIGNAL(newStatus()),this,SLOT(updateSpectometerInfo()));
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

void MainWindow::initTable()
{
    connect(&moveTable,SIGNAL(tableConnected()),this,SLOT(setTableIsConnected()));
    connect(&moveTable,SIGNAL(tableDisconnected()),this,SLOT(setTableIsDisconnected()));
    connect(&moveTable,SIGNAL(statusUpdated()),this,SLOT(updateStatusInfo()));
    connect(&moveTable,SIGNAL(boundingWarning()),this,SLOT(showWarningBox()));
    updateStatusInfo();

    serialPortUpdater.setInterval(3000);
    connect(&serialPortUpdater,SIGNAL(timeout()),this,SLOT(updateComCheckBox()));
    serialPortUpdater.start();

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
}

void MainWindow::switchSpectrometerOnUi()
{
    updateComCheckBox();
    ui->spectrometerComPortLabel->setText("COM"+QString::number(spectometerLib.deviceInfo.iRadugaPort));
    ui->connectSpecButton->setText("Отключиться");
    ui->connectionSpectIndicator->setState(true);
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
        if(spectometerLib.getAvailableDevicesCount() >= 1){
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


void MainWindow::on_backToCenterButton_clicked()
{
    if(moveTable.status.isConnected){
        //--
    }
}

void MainWindow::on_gotoButton_clicked()
{
    if(moveTable.status.isConnected){ 
        moveTable.moveTo(ui->XBox->value(),ui->YBox->value());
    }
}

void MainWindow::setTableIsConnected()
{
    ui->connectionTableIndicator->setState(true);
    ui->comPortsBox->setEnabled(false);
}

void MainWindow::setTableIsDisconnected()
{
    ui->connectionTableIndicator->setState(false);
    ui->comPortsBox->setEnabled(true);
}

void MainWindow::updateStatusInfo()
{
    ui->tableX->setText(QString::number(moveTable.status.X));
    ui->tableY->setText(QString::number(moveTable.status.Y));
    ui->tableStatusLabel->setText(moveTable.status.idle);

    ui->absoluteRadio->setEnabled(moveTable.status.idle == "Idle");
    ui->relativeRadio->setEnabled(moveTable.status.idle == "Idle");
    ui->gotoButton->setEnabled(moveTable.status.idle == "Idle");
    ui->XBox->setEnabled(moveTable.status.idle == "Idle");
    ui->YBox->setEnabled(moveTable.status.idle == "Idle");


    ui->endPointXIndicator->setState(moveTable.status.PnX);
    ui->endPointYIndicator->setState(moveTable.status.PnY);

    if(moveTable.status.positionMode == TableStatus::Absolute){
        ui->coordsModLabel->setText("Абсолютно");
    }else{
        ui->coordsModLabel->setText("Онтносительно");
    }
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
