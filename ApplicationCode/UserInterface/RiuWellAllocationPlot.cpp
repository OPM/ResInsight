/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RiuWellAllocationPlot.h"

#include "RiaApplication.h"

#include "RimContextCommandBuilder.h"
#include "RimTofAccumulatedPhaseFractionsPlot.h"
#include "RimTotalWellAllocationPlot.h"
#include "RimWellAllocationPlot.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"

#include "RiuContextMenuLauncher.h"
#include "RiuNightchartsWidget.h"
#include "RiuPlotObjectPicker.h"

#include "cvfColor3.h"

#include <QBoxLayout>
#include <QContextMenuEvent>
#include <QLabel>
#include <QMenu>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellAllocationPlot::RiuWellAllocationPlot( RimWellAllocationPlot* plotDefinition, QWidget* parent )
    : RiuWellLogPlot( plotDefinition, parent )
{
    Q_ASSERT( m_plotDefinition );

    QVBoxLayout* mainLayout = new QVBoxLayout();
    this->setLayout( mainLayout );
    this->layout()->setMargin( 0 );
    this->layout()->setSpacing( 2 );

    new RiuPlotObjectPicker( m_plotTitle, m_plotDefinition );

    mainLayout->addWidget( m_plotTitle, 0, Qt::AlignCenter );

    auto plotWidgetsLayout = new QHBoxLayout();
    auto rightColumnLayout = new QVBoxLayout();

    mainLayout->addLayout( plotWidgetsLayout );
    plotWidgetsLayout->addLayout( rightColumnLayout );

    m_legendWidget                            = new RiuNightchartsWidget( this );
    RimWellAllocationPlot* wellAllocationPlot = dynamic_cast<RimWellAllocationPlot*>( m_plotDefinition.p() );
    new RiuPlotObjectPicker( m_legendWidget, wellAllocationPlot->plotLegend() );

    caf::CmdFeatureMenuBuilder menuBuilder;
    menuBuilder << "RicShowTotalAllocationDataFeature";
    new RiuContextMenuLauncher( m_legendWidget, menuBuilder );

    rightColumnLayout->addWidget( m_legendWidget );
    m_legendWidget->showPie( false );

    QWidget* totalFlowAllocationWidget = wellAllocationPlot->totalWellFlowPlot()->createViewWidget( this );
    new RiuPlotObjectPicker( totalFlowAllocationWidget, wellAllocationPlot->totalWellFlowPlot() );
    new RiuContextMenuLauncher( totalFlowAllocationWidget, menuBuilder );

    rightColumnLayout->addWidget( totalFlowAllocationWidget, Qt::AlignTop );
    rightColumnLayout->addWidget( wellAllocationPlot->tofAccumulatedPhaseFractionsPlot()->createViewWidget( this ),
                                  Qt::AlignTop );
    rightColumnLayout->addStretch();

    QWidget* wellFlowWidget = m_plotDefinition->createPlotWidget();

    plotWidgetsLayout->addWidget( wellFlowWidget );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellAllocationPlot::~RiuWellAllocationPlot() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellAllocationPlot::showLegend( bool doShow )
{
    if ( doShow )
        m_legendWidget->show();
    else
        m_legendWidget->hide();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellAllocationPlot::addLegendItem( const QString& name, const cvf::Color3f& color, float value )
{
    QColor sliceColor( color.rByte(), color.gByte(), color.bByte() );

    m_legendWidget->addItem( name, sliceColor, value );
    m_legendWidget->updateGeometry();
    m_legendWidget->update();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellAllocationPlot::clearLegend()
{
    m_legendWidget->clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuWellAllocationPlot::minimumSizeHint() const
{
    return QSize( 0, 100 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellAllocationPlot::contextMenuEvent( QContextMenuEvent* event )
{
    QMenu                      menu;
    caf::CmdFeatureMenuBuilder menuBuilder;

    menuBuilder << "RicShowContributingWellsFromPlotFeature";

    menuBuilder.appendToMenu( &menu );

    if ( menu.actions().size() > 0 )
    {
        menu.exec( event->globalPos() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuWellAllocationPlot::sizeHint() const
{
    return QSize( 0, 0 );
}
