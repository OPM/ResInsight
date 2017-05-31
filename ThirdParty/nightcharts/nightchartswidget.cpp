#include "nightchartswidget.h"
#include "nightcharts.h"
NightchartsWidget::NightchartsWidget(QWidget *parent) :
    QWidget(parent)
{
    clear();
}
void NightchartsWidget::setType(Nightcharts::type t)
{
    _chart.setType(t);
}
void NightchartsWidget::clear()
{
    _chart = Nightcharts();
    _chart.setType(Nightcharts::Histogramm);
    _chart.setLegendType(Nightcharts::Vertical);

    _margin_left = 16;
    _margin_top = 16;

}

void NightchartsWidget::paintEvent(QPaintEvent * e)
{
    QWidget::paintEvent(e);
    if(!_chart.pieceCount()) return ;
    QPainter painter;
    QFont font;
    painter.begin(this);
    int w = (this->width() - _margin_left - 150);
    int h = (this->height() - _margin_top - 100);
    int size = (w<h)?w:h;
    _chart.setCords(_margin_left, _margin_top,size, size);


    _chart.draw(&painter);
    _chart.drawLegend(&painter);
    //painter.end();
}

void NightchartsWidget::addItem(QString name, QColor color, float value)
{
    _chart.addPiece(name,color,value);
}
