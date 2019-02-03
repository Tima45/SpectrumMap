#include "samplescanner.h"

SampleScanner::SampleScanner(QObject *parent) : QObject(parent)
{
    connect(this,SIGNAL(moveToNext()),this,SLOT(moveingToPos()),Qt::QueuedConnection);
    connect(&positionChekerTimer,SIGNAL(timeout()),this,SLOT(checkPosition()),Qt::QueuedConnection);
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
    resultMap.clear();

    this->width = width;
    this->height = height;
    this->stride = stride;
    this->timeMs = timeMs;

    currentX = width/2.0;
    currentY = height/2.0;

    moveTable->setAbsolute();

    l.lockForRead();
    if(continueScanning){
        emit moveToNext();
    }
    l.unlock();

    //-----/*
    l.lockForRead();
    while(continueScanning){

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

    if(deviceLib->info.sSPK_REALTIME >= timeMs){
        deviceLib->info.sSPK_REALTIME = 0; //WARNING: может быть ошибка меняю в разных потоках

        SpectrumType * newSpectrum = new SpectrumType(deviceLib->spectrum);
        resultMap.insert(QPointF(currentX,currentY),newSpectrum);

        qDebug() << "scanning finished";
        deviceLib->stopScanSpectrum();

        currentX -= stride;
        if(currentX <= -width/2.0){
            currentY -= stride;
            currentX = width/2.0;
            if(currentY < -height/2.0){             //TODO: последний ряд не сканируется
                emit scanningStatus(QString("Сканирование закончено."));
                emit scanningFinished();
                stopAll();
                return;
            }
        }
        emit moveToNext();
    }
}

void SampleScanner::moveingToPos()
{
    moveTable->moveTo(currentX,currentY,500);
    positionChekerTimer.start(200);
    emit scanningStatus(QString("Смещаемся к следующей точке X%1 Y%2").arg(currentX).arg(currentY));
}

void SampleScanner::checkPosition()
{
    static int counter = 0;
    counter++;
    if(moveTable->status.X == currentX && moveTable->status.Y == currentY){
        counter = 0;
        positionChekerTimer.stop();
        if(!deviceLib->isScanning){
            deviceLib->startScanSpectrum();
            emit scanningStatus(QString("Сканирование... текущее положение X%1 Y%2").arg(currentX).arg(currentY));
        }else{
            stopAll();
            emit errorWhileScanning("Детектор и так уже набирает спектр :(");
        }
    }
    if(counter == 50000/positionChekerTimer.interval()){     //50sec.. too long!
        counter = 0;
        stopAll();
        emit scanningStatus(QString("Сканирование... не удалося").arg(currentX).arg(currentY));
        emit errorWhileScanning("Что-то долго едем... Возможно косяк?");
    }
}

void SampleScanner::stopAll()
{
    deviceLib->stopScanSpectrum();
    positionChekerTimer.stop();
    l.lockForWrite();
    continueScanning = false;
    l.unlock();
}
