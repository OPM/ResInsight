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
    std::set<QwtPlot::Axis> allAxes = {QwtPlot::xBottom, QwtPlot::yLeft, QwtPlot::xTop, QwtPlot::yRight};

    int fontPointSize = RiaApplication::instance()->preferences()->defaultPlotFontSize() - 1;

    for ( QwtPlot::Axis axis : allAxes )
    {
        QwtText text = this->axisTitle( axis );
        QFont   font = text.font();
        font.setPixelSize(
            RiaFontCache::pointSizeToPixelSize( RiaApplication::instance()->preferences()->defaultPlotFontSize() ) );
        text.setFont( font );
        this->setAxisTitle( axis, text );

        QFont valuesFont = this->axisFont( axis );
        valuesFont.setPixelSize( font.pixelSize() );
        this->setAxisFont( axis, valuesFont );
    }

    if ( legend() )
    {
        auto font = legend()->font();
        font.setPointSize( fontPointSize );
        legend()->setFont( font );
    }

    QwtText titleText = this->title();
    QFont   font      = titleText.font();

    font.setPointSize( fontPointSize + 3 );
    titleText.setFont( font );
    this->setTitle( titleText );

    if ( replot )
    {
        this->replot();
    }
}
