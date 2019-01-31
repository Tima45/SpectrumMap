#include "indicator.h"

Indicator::Indicator(QWidget *parent, Qt::WindowFlags f) : QLabel(parent)
{
    Q_UNUSED(f);
    onPix = QPixmap(":/picures/indicatorOn.png");
    onPix2 = QPixmap(":/picures/indicatorOn2.png");
    offPix = QPixmap(":/picures/indicatorOff.png");
    loadingMov = new QMovie(":/picures/indicatorLoading.gif");
    loadingMov->start();
    this->setPixmap(offPix);
    this->setToolTip("Отключено");
    this->setScaledContents(true);
}

bool Indicator::isTernedOn()
{
    return value;
    delete loadingMov;
}

void Indicator::setState(bool value)
{
    this->value = value;
    if(value){
        this->setPixmap(onPix);
        this->setToolTip("Подключено");
    }else{
        this->setPixmap(offPix);
        this->setToolTip("Отключено");
    }
}

void Indicator::blink()
{
    if(value){
        if(rand()%2 == 0){
            this->setPixmap(onPix);
        }else{
            this->setPixmap(onPix2);
        }
    }
}

void Indicator::setLoading()
{
    if(this->movie() == nullptr){
        this->clear();
        this->setMovie(loadingMov);
    }
}
