#ifndef SAMPLESCANNER_H
#define SAMPLESCANNER_H

#include <QObject>
#include <QReadWriteLock>
#include <QDebug>
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
    QMap<QPointF,SpectrumType*> resultMap;
    void stopAll();
signals:
    void errorWhileScanning(QString);
    void scanningStatus(QString);
    void scanningFinished();
    void moveToNext();
public slots:
    void startScan(double width,double height,double stride,int timeMs);
private slots:
    void getSpectrum();
    void moveingToPos();
    void checkPosition();
private:
    QHPGDeviceLib *deviceLib = nullptr;
    MoveTable *moveTable = nullptr;

    QTimer positionChekerTimer;
    QTimer scanTimer;

    double width;
    double height;
    double stride;
    int timeMs;





};

#endif // SAMPLESCANNER_H
