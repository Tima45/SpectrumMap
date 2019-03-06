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
    void updateColorMap();

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

    void updateStatusScanning(QString status);

    void drawSpectrum();

    void updateColorMap(QPointF point, SpectrumType* spectrum);

    void on_savePicButton_clicked();

    void on_openPicButton_clicked();

    void showLoadedOnMap();


    void on_groupBox_4_clicked(bool checked);

    void toggleVisibleOnLayout(QLayout *lay, bool v);

    void on_groupBox_3_clicked(bool checked);

    void on_groupBox_2_clicked(bool checked);

    void on_groupBox_clicked(bool checked);

    void handleClick(QMouseEvent *);

    void on_shownEnergyBox_valueChanged(double arg1);

    void on_energyIntervalBox_valueChanged(double arg1);

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
    SampleScanner *scanner = nullptr;
    //--------------
    QVector<double> x;
    QVector<double> energy;
    QVector<double> y;
    QCPGraph *spectrumGraph = nullptr;
    QCPItemLine *energyLine = nullptr;
    QCPItemLine *energyLineLeft = nullptr;
    QCPItemLine *energyLineRgiht = nullptr;
    void setLineKey(QCPItemLine *line,double key);

    QCPColorScale *colorScale = nullptr;
    QCPColorMap *colorMap = nullptr;
    QCPItemTracer *spetrometerPos = nullptr;
    QCPItemEllipse *el = nullptr;
    void setPostEl(double x,double y);
    //----------------
    void loadSettings();
    int toChanal(double value);
    double toEnergy(int c);
    double countSummOnInterval(SpectrumType* spectrum);
    double C0,C1;
    bool freeIntaraction = false;
    bool autoRescaleData = true;

};


#endif // MAINWINDOW_H
