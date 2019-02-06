#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QTimer>
#include <QRegExp>
#include <QSerialPortInfo>
#include "movetable.h"
#include "qhpgdevicelib.h"
#include "plot/qcustomplot.h"
#include <QThread>
#include <QReadWriteLock>
#include "samplescanner.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
signals:
    void startScanning(double,double,double,int);

private slots:
    void switchSpectrometerOffUi();
    void switchSpectrometerOnUi();
    void on_setHvButton_clicked();
    void updateSpectometerInfo();
    void on_connectSpecButton_clicked();

    void on_tableTableButton_clicked();
    void on_tableFindZeroButton_clicked();
    void on_gotoButton_clicked();
    void setTableIsConnected();
    void setTableIsDisconnected();
    void updateStatusInfo();

    void on_resetAlarmButton_clicked();

    void on_softResetBUtton_clicked();

    void on_setZeroButton_clicked();


    void on_absoluteRadio_toggled(bool checked);

    void on_relativeRadio_toggled(bool checked);

    void updateComCheckBox();

    void showWarningBox();

    void on_startScanButton_clicked();

    void stopScanning();

    void switchButtons(bool v);

    void switchScanButtons(bool v);

    void showMessageBox(QString text);

    void thenScanningFinished();

private:
    Ui::MainWindow *ui;
    void closeEvent(QCloseEvent *event);
    //-------------
    void initPlot();
    void initScanner();
    void initTable();
    void initSpectrometerLib();
    //-------------
    QHPGDeviceLib spectometerLib;
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);
    bool hvOn = false;
    void seHvIndicatorValue(int value);
    //------------
    MoveTable moveTable;
    QTimer serialPortUpdater;
    bool isScanning  = false;
    //--------------
    SampleScanner *scanner;
    QReadWriteLock l;




};


#endif // MAINWINDOW_H
