/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RiuWellLogTrack.h"

#include "RiaDefines.h"
#include "RiaPlotDefines.h"
#include "RimWellLogCurve.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogTrack.h"

#include "RigWellLogCurveData.h"

#include "RiuGuiTheme.h"
#include "RiuPlotCurve.h"
#include "RiuPlotCurveInfoTextProvider.h"
#include "RiuQwtCurvePointTracker.h"
#include "RiuQwtPlotTools.h"

#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_scale_draw.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_widget.h"

#include <QWheelEvent>

class RiuWellLogCurvePointTracker : public RiuQwtCurvePointTracker
{
public:
    RiuWellLogCurvePointTracker( QwtPlot* plot, RiuPlotCurveInfoTextProvider* curveInfoTextProvider, RimWellLogTrack* track )
        : RiuQwtCurvePointTracker( plot, false, curveInfoTextProvider )
        , m_wellLogTrack( track )
    {
    }

protected:
    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    QwtText trackerText( const QPoint& pos ) const override
    {
        QwtText txt;

        if ( m_plot )
        {
            QwtAxisId relatedYAxis( QwtAxis::YLeft, 0 );
            QwtAxisId relatedXAxis( QwtAxis::XTop, 0 );

            QString curveInfoText;
            QString depthAxisValueString;
            QString xAxisValueString;
            QPointF closestPoint =
                closestCurvePoint( pos, &curveInfoText, &xAxisValueString, &depthAxisValueString, &relatedXAxis, &relatedYAxis );
            if ( !closestPoint.isNull() )
            {
                QString str;

                RimWellLogPlot* wlp = nullptr;
                m_wellLogTrack->firstAncestorOfType( wlp );

                if ( wlp && wlp->depthOrientation() == RimDepthTrackPlot::DepthOrientation::HORIZONTAL )
                {
                    str = QString( "%1\nDepth: %2" ).arg( depthAxisValueString ).arg( xAxisValueString );
                }
                else
                {
                    str = QString( "%1\nDepth: %2" ).arg( xAxisValueString ).arg( depthAxisValueString );
                }

                if ( !curveInfoText.isEmpty() )
                {
                    str = QString( "%1:\n" ).arg( curveInfoText ) + str;
                }

                txt.setText( str );
            }

            updateClosestCurvePointMarker( closestPoint, relatedXAxis, relatedYAxis );
        }

        auto color = RiuGuiTheme::getColorByVariableName( "markerColor" );
        txt.setColor( color );

        return txt;
    }

private:
    caf::PdmPointer<RimWellLogTrack> m_wellLogTrack;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class WellLogCurveInfoTextProvider : public RiuPlotCurveInfoTextProvider
{
public:
    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    QString curveInfoText( RiuPlotCurve* riuCurve ) const override
    {
        if ( riuCurve )
        {
            RimWellLogCurve* wlCurve = dynamic_cast<RimWellLogCurve*>( riuCurve->ownerRimCurve() );
            if ( wlCurve )
            {
                return QString( "%1" ).arg( wlCurve->curveName() );
            }
        }

        return "";
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    QString additionalText( RiuPlotCurve* curve, int sampleIndex ) const override
    {
        if ( curve )
        {
            std::vector<std::pair<QString, double>> propertyNameValues;

            auto* sourceCurve = curve->ownerRimCurve();

            if ( sourceCurve )
            {
                auto annotationCurves = sourceCurve->annotationCurves();
                for ( auto annotationCurve : annotationCurves )
                {
                    RimDepthTrackPlot* depthTrackPlot = nullptr;
                    annotationCurve->firstAncestorOfType( depthTrackPlot );
                    if ( depthTrackPlot )
                    {
                        auto [xValue, yValue] = curve->sample( sampleIndex );

                        auto depth = depthTrackPlot->depthOrientation() == RimDepthTrackPlot::DepthOrientation::VERTICAL
                                         ? yValue
                                         : xValue;

                        auto propertyValue = annotationCurve->closestYValueForX( depth );

                        // Use template to get as short label as possible. The default curve name will often
                        // contain too much and redundant information.
                        QString templateText = RiaDefines::namingVariableResultName();
                        auto    resultName   = annotationCurve->annotationCurveName( templateText );

                        propertyNameValues.push_back( std::make_pair( resultName, propertyValue ) );
                    }
                }
            }

            if ( !propertyNameValues.empty() )
            {
                QString txt;
                for ( const auto& [name, value] : propertyNameValues )
                {
                    txt += QString( "%1 : %2\n" ).arg( name ).arg( value );
                }

                return txt;
            }
        }

        return {};
    }
};
static WellLogCurveInfoTextProvider wellLogCurveInfoTextProvider;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellLogTrack::RiuWellLogTrack( RimWellLogTrack* track, QWidget* parent /*= nullptr */ )
    : RiuQwtPlotWidget( track, parent )
{
    RimWellLogPlot* wlp = nullptr;
    track->firstAncestorOfType( wlp );

    bool isVertical = ( wlp && wlp->depthOrientation() == RimDepthTrackPlot::DepthOrientation::VERTICAL );
    setAxisEnabled( QwtAxis::YLeft, true );
    setAxisEnabled( QwtAxis::YRight, false );
    setAxisEnabled( QwtAxis::XTop, !isVertical );
    setAxisEnabled( QwtAxis::XBottom, isVertical );

    new RiuWellLogCurvePointTracker( this->qwtPlot(), &wellLogCurveInfoTextProvider, track );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellLogTrack::~RiuWellLogTrack()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogTrack::setAxisEnabled( QwtAxis::Position axis, bool enabled )
{
    RiuPlotAxis plotAxis = RiuPlotAxis( RiuQwtPlotTools::fromQwtPlotAxis( axis ) );
    RiuQwtPlotWidget::enableAxis( plotAxis, enabled );

    if ( enabled )
    {
        // Align the canvas with the actual min and max values of the curves
        qwtPlot()->axisScaleEngine( axis )->setAttribute( QwtScaleEngine::Floating, true );
        setAxisScale( plotAxis, 0.0, 100.0 );
        qwtPlot()->axisScaleDraw( axis )->setMinimumExtent( axisExtent( plotAxis ) );

        qwtPlot()->axisWidget( axis )->setMargin( 0 );
    }

    setAxisTitleEnabled( plotAxis, enabled );
}
