#include "samplescanner.h"

SampleScanner::SampleScanner(QObject *parent) : QObject(parent)
{

}

SampleScanner::~SampleScanner()
{

}

void SampleScanner::setDevices(QHPGDeviceLib *deviceLib, MoveTable *moveTable)
{
    this->deviceLib = deviceLib;
    this->moveTable = moveTable;
    connect(this->deviceLib,SIGNAL(newSpectrum()),this,SLOT(getSpectrum()),Qt::ConnectionType::QueuedConnection);
}

void SampleScanner::startScan(double width, double height, double stride, int timeMs)
{
    if(deviceLib == nullptr || moveTable == nullptr){
        qDebug() << "Devices not setuped";
        return;
    }
    if(!deviceLib->isConnected() || !moveTable->isConnected()){
        emit errorWhileScanning("Устройства не подключены.");
        return;
    }

    /*if(deviceLib->extraInfo.sDAC1_CURVAL != 1450){
        emit errorWhileScanning("Нет высокого напряжения.");
        return;
    }*/
    if(moveTable->status.idle != "Idle" || deviceLib->isScanning){
        emit errorWhileScanning("Устройства не готовы.");
        return;
    }
    if(moveTable->status.X != 0 || moveTable->status.Y != 0){
        emit errorWhileScanning("Стол дожен находиться в центре.");
        return;
    }

    emit scanningStatus("Едем в начало координат образца...");
    currentX = width/2.0;
    currentY = height/2.0;

    moveTable->setAbsolute();
    l.lockForRead();
    while(continueScanning){

        //TODO: переделать всё через сигналы слоты. текущий поток спит и не сканирует
        l.unlock();

        moveTable->moveTo(currentX,currentY,500);
        this->thread()->msleep(100);
        while(moveTable->status.X != currentX && moveTable->status.Y != currentY){
            this->thread()->msleep(100);
            qDebug() << "waiting";
        }
        emit scanningStatus(QString("Сканирование... текущее положение X%1 Y%2").arg(currentX).arg(currentY));
        deviceLib->startScanSpectrum();
        this->thread()->msleep(timeMs);
        deviceLib->stopScanSpectrum();
        emit scanningStatus(QString("Смещаемся к следующей точке..."));
        currentX -= stride;
        if(currentX <= -width/2.0){
            currentY -= stride;
            currentX = width/2.0;
            if(currentY < -height/2.0){
                emit scanningStatus(QString("Сканирование закончено."));
                emit scanningFinished();

                l.lockForWrite();
                continueScanning = false;
                l.unlock();
            }
        }
        l.lockForRead();
    }
    l.unlock();
}

void SampleScanner::getSpectrum()
{

}
