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

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void switchSpectrometerOffUi();
    void switchSpectrometerOnUi();
    void on_setHvButton_clicked();
    void updateSpectometerInfo();
    void on_connectSpecButton_clicked();

    void on_tableTableButton_clicked();
    void on_tableFindZeroButton_clicked();
    void on_backToCenterButton_clicked();
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

private:
    Ui::MainWindow *ui;
    //-------------
    void initPlot();
    //-------------
    QHPGDeviceLib spectometerLib;
    void initSpectrometerLib();
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);
    bool hvOn = false;
    void seHvIndicatorValue(int value);
    //------------
    void initTable();
    MoveTable moveTable;

    QTimer serialPortUpdater;


};


#endif // MAINWINDOW_H
