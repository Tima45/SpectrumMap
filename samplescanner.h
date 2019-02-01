#ifndef SAMPLESCANNER_H
#define SAMPLESCANNER_H

#include <QObject>
#include <QReadWriteLock>
#include "movetable.h"
#include "qhpgdevicelib.h"

class SampleScanner : public QObject
{
    Q_OBJECT
public:
    explicit SampleScanner(QObject *parent = 0);
    ~SampleScanner();
    void setDevices(QHPGDeviceLib *deviceLib,MoveTable *moveTable);
    double currentX = 0;
    double currentY = 0;
    bool continueScanning = false;
    QReadWriteLock l;
signals:
    void errorWhileScanning(QString);
    void scanningStatus(QString);
    void scanningFinished();
public slots:
    void startScan(double width,double height,double stride,int timeMs);
private slots:
    void getSpectrum();
private:
    QHPGDeviceLib *deviceLib = nullptr;
    MoveTable *moveTable = nullptr;

};

#endif // SAMPLESCANNER_H
