// Based on the example scalepicker from the Qwt/examples/event_filter

#include "RiuQwtScalePicker.h"
#include "RiuQwtPlotWidget.h"

#include <QMouseEvent>

#include <qwt_plot.h>
#include <qwt_scale_widget.h>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtScalePicker::RiuQwtScalePicker( QwtPlot* plot )
    : QObject( plot )
{
    for ( uint i = 0; i < QwtPlot::axisCnt; i++ )
    {
        QwtScaleWidget* scaleWidget = plot->axisWidget( i );
        if ( scaleWidget ) scaleWidget->installEventFilter( this );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuQwtScalePicker::eventFilter( QObject* object, QEvent* event )
{
    if ( event->type() == QEvent::MouseButtonPress )
    {
        QwtScaleWidget* scaleWidget = qobject_cast<QwtScaleWidget*>( object );
        if ( scaleWidget )
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>( event );
            Q_EMIT( mouseClicked( scaleWidget, mouseEvent->pos() ) );

            return true;
        }
    }

    return QObject::eventFilter( object, event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiuQwtScalePicker::axisValueAtPosition( const QwtScaleWidget* scale, const QPoint& pos )
{
    QRect rect = scale->rect();

    int margin = 10; // 10 pixels tolerance
    rect.setRect( rect.x() - margin, rect.y() - margin, rect.width() + 2 * margin, rect.height() + 2 * margin );

    if ( rect.contains( pos ) ) // No click on the title
    {
        // translate the position in a value on the scale

        double value = 0.0;
        int    axis  = -1;

        const QwtScaleDraw* sd = scale->scaleDraw();
        switch ( scale->alignment() )
        {
            case QwtScaleDraw::LeftScale:
            {
                value = sd->scaleMap().invTransform( pos.y() );
                axis  = QwtPlot::yLeft;
                break;
            }
            case QwtScaleDraw::RightScale:
            {
                value = sd->scaleMap().invTransform( pos.y() );
                axis  = QwtPlot::yRight;
                break;
            }
            case QwtScaleDraw::BottomScale:
            {
                value = sd->scaleMap().invTransform( pos.x() );
                axis  = QwtPlot::xBottom;
                break;
            }
            case QwtScaleDraw::TopScale:
            {
                value = sd->scaleMap().invTransform( pos.x() );
                axis  = QwtPlot::xTop;
                break;
            }
        }
        return value;
    }
    return std::numeric_limits<double>::infinity();
}
