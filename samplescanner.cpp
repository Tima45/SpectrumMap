#include "samplescanner.h"
#include "math.h"


SampleScanner::SampleScanner(QObject *parent) : QObject(parent)
{
    connect(this,SIGNAL(moveToNext()),this,SLOT(moveingToPos()));
    connect(&positionChekerTimer,SIGNAL(timeout()),this,SLOT(checkPosition()));
    connect(&scanTimer,SIGNAL(timeout()),this,SLOT(getSpectrum()));
    scanTimer.setSingleShot(true);
    positionChekerTimer.setInterval(200);
}

SampleScanner::~SampleScanner()
{

}

void SampleScanner::setDevices(QHPGDeviceLib *deviceLib, MoveTable *moveTable)
{
    this->deviceLib = deviceLib;
    this->moveTable = moveTable;
    //connect(this->deviceLib,SIGNAL(newSpectrum()),this,SLOT(getSpectrum()),Qt::ConnectionType::QueuedConnection);
}

void SampleScanner::clearSpectrumMap()
{
    for(int i = 0; i < resultMap.count(); i++){
        delete resultMap.at(i).second;
    }

    resultMap.clear();
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

    if(deviceLib->extraInfo.sDAC1_CURVAL != 1450){
        emit errorWhileScanning("Нет высокого напряжения.");
        return;
    }

    if(moveTable->status.idle != "Idle" || deviceLib->isScanning){
        emit errorWhileScanning("Устройства не готовы.");
        return;
    }
    if(moveTable->status.X != 0 || moveTable->status.Y != 0){
        emit errorWhileScanning("Стол дожен находиться в центре.");
        return;
    }

    timeStart = QDateTime::currentDateTime();


    clearSpectrumMap();

    moveTable->setAbsolute();

    l.lockForRead();
    this->width = width;
    this->height = height;
    this->stride = stride;
    this->timeMs = timeMs;

    revers = -1.0;

    currentX = width/2.0;
    currentY = -height/2.0;
    positionChekerTimer.start();
    if(continueScanning){
        emit moveToNext();
    }
    l.unlock();

}




void SampleScanner::getSpectrum()
{
    if(continueScanning){
    qDebug() << deviceLib->info.sSPK_REALTIME << timeMs;

        deviceLib->stopScanSpectrum();

        deviceLib->info.sSPK_REALTIME = 0;

        SpectrumType *newSpectrum = new SpectrumType(deviceLib->spectrum);

        QPointF p = QPointF(-currentX,-currentY);


        QPair<QPointF,SpectrumType*> pair;
        pair.first = p;
        pair.second = newSpectrum;




        resultMap.append(pair);

        emit newResult(p,newSpectrum);

        qDebug() << resultMap.count() << "Количество точек в спектре";

        currentX += revers*stride;
        if(currentX > width/2.0 || currentX < -width/2.0){
            currentX = revers*width/2.0;
            revers *= -1.0;

            currentY += stride;
            if(currentY > height/2.0){
                stopAll();
                emit scanningStatus(QString("Сканирование закончено."));
                emit scanningFinished();
                return;
            }
        }
        positionChekerTimer.start();
        emit moveToNext();
    }
}

void SampleScanner::moveingToPos()
{
    if(continueScanning){
        moveTable->moveTo(currentX,currentY);
        emit scanningStatus(QString("Смещаемся к следующей точке X%1 Y%2").arg(-currentX).arg(-currentY));
    }
}

void SampleScanner::checkPosition()
{
    static int counter = 0;
    counter++;
    qDebug() << counter;
    if(moveTable->status.X == currentX && moveTable->status.Y == currentY){
        counter = 0;
        positionChekerTimer.stop();
        if(!deviceLib->isScanning){
            deviceLib->startScanSpectrum();

            scanTimer.start(timeMs);
            emit scanningStatus(QString("Сканирование... текущее положение X%1 Y%2").arg(-currentX).arg(-currentY));
        }else{
            stopAll();
            emit errorWhileScanning("Детектор и так уже набирает спектр :(");
        }
        return;
    }
    if(counter == 10000/positionChekerTimer.interval()){     //10sec.. too long!
        counter = 0;
        //stopAll();
        emit moveToNext();
        qDebug() << "Пмнём еще разок";
        //emit scanningStatus(QString("Сканирование... не удалося").arg(currentX).arg(currentY));
        //emit errorWhileScanning("Что-то долго едем... Возможно косяк?");
        return;
    }
}

void SampleScanner::stopAll()
{
    timeStop = QDateTime::currentDateTime();
    deviceLib->stopScanSpectrum();
    positionChekerTimer.stop();
    l.lockForWrite();
    continueScanning = false;
    l.unlock();
}
