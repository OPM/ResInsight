#ifndef NIGHTCHARTSWIDGET_H
#define NIGHTCHARTSWIDGET_H

#include <QWidget>
#include <QPaintEvent>
#include "nightcharts.h"
class NightchartsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NightchartsWidget(QWidget *parent = 0);
    void addItem(QString name, QColor color, float value);
    void setType(Nightcharts::type type);
    void clear();
protected:
    virtual void paintEvent(QPaintEvent * e);
private:
    Nightcharts _chart;
    int _margin_left;
    int _margin_top;

};

#endif // NIGHTCHARTSWIDGET_H
