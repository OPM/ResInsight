// Based on the example scalepicker from the Qwt/examples/event_filter

#pragma once

#include <QObject>
#include <QRect>

class QwtPlot;
class QwtScaleWidget;

class RiuQwtScalePicker : public QObject
{
    Q_OBJECT
public:
    explicit RiuQwtScalePicker(QwtPlot *plot);
    
    bool eventFilter( QObject *, QEvent * ) override;

Q_SIGNALS:
    void clicked( int axis, double value );

private:
    void mouseClicked( const QwtScaleWidget *, const QPoint & );
};
