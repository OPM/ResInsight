/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RimEnsembleFractureStatisticsPlot.h"

#include "RigEnsembleFractureStatisticsCalculator.h"
#include "RimEnsembleFractureStatistics.h"
#include "RimPlot.h"
#include "RimProject.h"

#include "RigHistogramData.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiComboBoxEditor.h"

#include "cvfAssert.h"

#include <cmath>

CAF_PDM_SOURCE_INIT( RimEnsembleFractureStatisticsPlot, "EnsembleFractureStatisticsPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleFractureStatisticsPlot::RimEnsembleFractureStatisticsPlot()
{
    CAF_PDM_InitObject( "Ensemble Fracture Statistics Plot", "", "", "A Plot of Ensemble Fracture Statistics" );

    CAF_PDM_InitFieldNoDefault( &m_ensembleFractureStatistics,
                                "EnsembleFractureStatistics",
                                "Ensemble Fracture Statistics");
    m_ensembleFractureStatistics.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_property, "Property", "Property" );

    m_plotLegendsHorizontal.uiCapability()->setUiHidden( true );

    setDefaults();

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleFractureStatisticsPlot::~RimEnsembleFractureStatisticsPlot()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFractureStatisticsPlot::setDefaults()
{
    std::vector<RimEnsembleFractureStatistics*> esfItems;
    RimProject::current()->descendantsIncludingThisOfType( esfItems );
    if ( !esfItems.empty() ) m_ensembleFractureStatistics = esfItems.front();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFractureStatisticsPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                          const QVariant&            oldValue,
                                                          const QVariant&            newValue )
{
    RimStatisticsPlot::fieldChangedByUi( changedField, oldValue, newValue );
    loadDataAndUpdate();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFractureStatisticsPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_ensembleFractureStatistics );
    uiOrdering.add( &m_property );

    bool showNumHistogramBins = true;
    RimStatisticsPlot::uiOrderingForHistogram( uiConfigName, uiOrdering, showNumHistogramBins );

    caf::PdmUiGroup* plotLayoutGroup = uiOrdering.addNewGroup( "Plot Layout" );
    RimStatisticsPlot::uiOrderingForPlotLayout( uiConfigName, *plotLayoutGroup );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimEnsembleFractureStatisticsPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                              bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options = RimStatisticsPlot::calculateValueOptions( fieldNeedingOptions, useOptionsOnly );

    if ( fieldNeedingOptions == &m_ensembleFractureStatistics )
    {
        std::vector<RimEnsembleFractureStatistics*> esfItems;
        RimProject::current()->descendantsIncludingThisOfType( esfItems );
        for ( auto item : esfItems )
        {
            options.push_back( caf::PdmOptionItemInfo( item->name(), item ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleFractureStatisticsPlot::hasStatisticsData() const
{
    return ( m_viewer && m_ensembleFractureStatistics );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigHistogramData RimEnsembleFractureStatisticsPlot::createStatisticsData() const
{
    RigHistogramData histogramData =
        RigEnsembleFractureStatisticsCalculator::createStatisticsData( m_ensembleFractureStatistics(),
                                                                       m_property(),
                                                                       m_numHistogramBins() );

    return histogramData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEnsembleFractureStatisticsPlot::createAutoName() const
{
    if ( m_ensembleFractureStatistics() == nullptr )
    {
        return "Undefined";
    }

    QStringList nameTags;
    nameTags += m_ensembleFractureStatistics()->name();
    nameTags += caf::AppEnum<RigEnsembleFractureStatisticsCalculator::PropertyType>::uiText( m_property() );

    return nameTags.join( ", " );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEnsembleFractureStatisticsPlot::createXAxisTitle() const
{
    if ( m_ensembleFractureStatistics() == nullptr ) return "";

    return caf::AppEnum<RigEnsembleFractureStatisticsCalculator::PropertyType>::uiText( m_property() );
}
