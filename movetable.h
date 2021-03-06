#ifndef MOVETABLE_H
#define MOVETABLE_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QRegExp>
#include "masterthread.h"
struct TableStatus{
    enum PositionMode{
        Relative,
        Absolute
    };
    bool isConnected = false;
    PositionMode positionMode = Absolute;
    QString idle;
    double X = 0;
    double Y = 0;
    double speed = 0;
    bool PnX = false;
    bool PnY = false;
};

class MoveTable : public MasterThread
{
    Q_OBJECT
public:
    static const double xMax;
    static const double yMax;
    static const double xMin;
    static const double yMin;
    explicit MoveTable(QObject *parent = 0);
    ~MoveTable();
    TableStatus status;
    QTimer tableStatusTimer;
    void setSerialPort(QString port);
    bool findingHome = false;
    int statusUpdateTime = 500;
signals:
    void tableConnected();
    void tableDisconnected();
    void statusUpdated();
    void boundingWarning();
public slots:
    void connectToTable();
    void readResponcse(QString s);
    void readErrorResponcse(QString s);
    void askStatus();
    void moveTo(double x, double y);
    void moveTo(double x, double y,double speed);
    void resetAlarm();
    void findZero();
    void softReset();
    void setZero();
    void setAbsolute();
    void setRelative();
    bool isConnected();
private:
    bool check(double x ,double y);
    QString serialPort = "";
    QRegExp r;
    static const int responseTimeMs = 1000;

};

#endif // MOVETABLE_H
