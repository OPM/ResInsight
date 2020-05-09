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
    std::set<QwtPlot::Axis> allAxes = { QwtPlot::xBottom, QwtPlot::yLeft, QwtPlot::xTop, QwtPlot::yRight };

    caf::FontTools::FontSize fontSize = RiaPreferences::current()->defaultPlotFontSize();

    int titleFontSize  = caf::FontTools::absolutePointSize( fontSize, caf::FontTools::RelativeSize::Large );
    int axisFontSize   = caf::FontTools::absolutePointSize( fontSize, caf::FontTools::RelativeSize::Medium );
    int legendFontSize = caf::FontTools::absolutePointSize( fontSize, caf::FontTools::RelativeSize::Small );

    QwtText titleText = this->title();
    QFont   font      = titleText.font();

    font.setPixelSize( caf::FontTools::pointSizeToPixelSize( titleFontSize ) );
    titleText.setFont( font );
    this->setTitle( titleText );

    for ( QwtPlot::Axis axis : allAxes )
    {
        QwtText text          = this->axisTitle( axis );
        QFont   axisTitleFont = text.font();
        axisTitleFont.setPixelSize( caf::FontTools::pointSizeToPixelSize( axisFontSize ) );
        text.setFont( axisTitleFont );
        this->setAxisTitle( axis, text );

        QFont valuesFont = this->axisFont( axis );
        valuesFont.setPixelSize( axisTitleFont.pixelSize() );
        this->setAxisFont( axis, valuesFont );
    }

    if ( legend() )
    {
        auto font = legend()->font();
        font.setPixelSize( caf::FontTools::pointSizeToPixelSize( legendFontSize ) );
        legend()->setFont( font );
    }

    if ( replot )
    {
        this->replot();
    }
}
