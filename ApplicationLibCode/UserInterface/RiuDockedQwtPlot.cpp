/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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
#include "RiuDockedQwtPlot.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "qwt_abstract_legend.h"
#include "qwt_text.h"

#include <QFont>
#include <set>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuDockedQwtPlot::RiuDockedQwtPlot( QWidget* parent /*= nullptr*/ )
    : QwtPlot( parent )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuDockedQwtPlot::applyFontSizes( bool replot /*= false*/ )
{
    std::set<QwtAxis::Position> allAxes = { QwtAxis::XBottom, QwtAxis::YLeft, QwtAxis::XTop, QwtAxis::YRight };

    caf::FontTools::FontSize fontSize = RiaPreferences::current()->defaultPlotFontSize();

    int titleFontSize  = caf::FontTools::absolutePointSize( fontSize, caf::FontTools::RelativeSize::Large );
    int axisFontSize   = caf::FontTools::absolutePointSize( fontSize, caf::FontTools::RelativeSize::Medium );
    int legendFontSize = caf::FontTools::absolutePointSize( fontSize, caf::FontTools::RelativeSize::Small );

    QwtText titleText = title();
    QFont   font      = titleText.font();

    font.setPointSize( titleFontSize );
    titleText.setFont( font );
    setTitle( titleText );

    for ( QwtAxis::Position axis : allAxes )
    {
        QwtText text          = axisTitle( axis );
        QFont   axisTitleFont = text.font();
        axisTitleFont.setPointSize( axisFontSize );
        text.setFont( axisTitleFont );
        setAxisTitle( axis, text );

        QFont valuesFont = axisFont( axis );
        valuesFont.setPointSize( axisTitleFont.pointSize() );
        setAxisFont( axis, valuesFont );
    }

    if ( legend() )
    {
        auto font = legend()->font();
        font.setPointSize( legendFontSize );
        legend()->setFont( font );
    }

    if ( replot )
    {
        this->replot();
    }
}
