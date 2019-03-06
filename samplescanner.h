#ifndef SAMPLESCANNER_H
#define SAMPLESCANNER_H

#include <QObject>
#include <QReadWriteLock>
#include <QDebug>
#include <QDateTime>
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
    QDateTime timeStart;
    QDateTime timeStop;
    QList<QPair<QPointF,SpectrumType*>> resultMap;
    void stopAll();
    double width = 50;
    double height = 50;
    double stride = 10;
    void clearSpectrumMap();

signals:
    void errorWhileScanning(QString);
    void scanningStatus(QString);
    void scanningFinished();
    void moveToNext();
    void newResult(QPointF,SpectrumType*);
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


    int timeMs;

    double revers = -1.0;





};

#endif // SAMPLESCANNER_H
