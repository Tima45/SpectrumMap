#include "movetable.h"

const double MoveTable::xMax = 63;
const double MoveTable::yMax = 63;
const double MoveTable::xMin = -63;
const double MoveTable::yMin = -63;

MoveTable::MoveTable(QObject *parent) : MasterThread(parent)
{
    connect(this,SIGNAL(response(QString)),this,SLOT(readResponcse(QString)));
    connect(this,SIGNAL(error(QString)),this,SLOT(readErrorResponcse(QString)));
    connect(&tableStatusTimer,SIGNAL(timeout()),this,SLOT(askStatus()));
    r = QRegExp("<(.*)\\|WPos:(.?\\d*\\.*\\d*),(.?\\d*\\.*\\d*),(.?\\d*\\.*\\d*)\\|FS:.?\\d*\\.*\\d*,.?\\d*\\.*\\d*(\\|Pn:([XYZ]*))?");
    tableStatusTimer.setInterval(500);
}

MoveTable::~MoveTable()
{

}

void MoveTable::setSerialPort(QString port)
{
    serialPort = port;
}

void MoveTable::connectToTable()
{
    if(!status.isConnected){
        transaction(serialPort,responseTimeMs,"?");
    }
}

void MoveTable::readResponcse(QString s)
{
    if(!status.isConnected){
        QStringList list = s.split("\r\n");
        if(list.count() >= 2){
            if(list.at(1) == "ok"){
                status.isConnected = true;
                emit tableConnected();
                tableStatusTimer.start();
            }
        }
    }
    qDebug() << s;
    if(r.indexIn(s) != -1){
        status.idle = r.cap(1);
        status.X = r.cap(2).toDouble();
        status.Y = r.cap(3).toDouble();
        QString pins = r.cap(6);
        status.PnX = false;
        status.PnY = false;
        for(int i = 0; i < pins.count(); i++){
            if(pins.at(i) == "X"){
                status.PnX = true;
            }
            if(pins.at(i) == "Y"){
                status.PnY = true;
            }
        }

        emit statusUpdated();
    }
}

void MoveTable::readErrorResponcse(QString s)
{
    qDebug() << Q_FUNC_INFO << s;
    status.isConnected = false;
    tableStatusTimer.stop();
    emit tableDisconnected();
}

void MoveTable::askStatus()
{
    if(status.isConnected){
        transaction(serialPort,responseTimeMs,"?");
    }
}


void MoveTable::moveTo(double x, double y)
{
    if(status.isConnected){
        if(status.positionMode == TableStatus::Relative){
            if(x+status.X >= xMax || y+status.Y >= yMax || x+status.X <= xMin || y+status.Y <= yMin){
                emit boundingWarning();
                return;
            }
        }
        if(status.positionMode == TableStatus::Absolute){
            if(x >= xMax || y >= yMax || x <= xMin || y <= yMin){
                emit boundingWarning();
                return;
            }
        }

        transaction(serialPort,responseTimeMs,QString("G0 X%1 Y%2").arg(x).arg(y));
    }
}

void MoveTable::resetAlarm()
{
    if(status.isConnected){
        transaction(serialPort,responseTimeMs,"$X");
    }
}

void MoveTable::findZero()
{
    if(status.isConnected){
        transaction(serialPort,responseTimeMs,"$H");
    }
}

void MoveTable::softReset()
{
    if(status.isConnected){
        char c = 0x18;
        transaction(serialPort,responseTimeMs,QString(c));
    }
}

void MoveTable::setZero()
{
    if(status.isConnected){
        transaction(serialPort,responseTimeMs,"G10 P0 L20 X0 Y0 Z0");
    }
}

void MoveTable::setAbsolute(){
    if(status.isConnected){
        status.positionMode = TableStatus::Absolute;
        transaction(serialPort,responseTimeMs,"G90");
    }
}

void MoveTable::setRelative(){
    if(status.isConnected){
        status.positionMode = TableStatus::Relative;
        transaction(serialPort,responseTimeMs,"G91");
    }
}


