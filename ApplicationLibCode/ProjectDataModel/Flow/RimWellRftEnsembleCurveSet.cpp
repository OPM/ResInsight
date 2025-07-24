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

#include "RimWellRftEnsembleCurveSet.h"

#include "RifReaderEnsembleStatisticsRft.h"

#include "Appearance/RimCurveSetAppearance.h"
#include "RimEclipseCase.h"
#include "RimProject.h"
#include "RimRegularLegendConfig.h"
#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"
#include "RimTools.h"
#include "RimWellRftPlot.h"

#include "cafPdmUiObjectHandle.h"
#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT( RimWellRftEnsembleCurveSet, "WellRftEnsembleCurveSet" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellRftEnsembleCurveSet::RimWellRftEnsembleCurveSet()
{
    CAF_PDM_InitObject( "Ensemble Curve Set", ":/EnsembleCurveSet16x16.png" );
    CAF_PDM_InitFieldNoDefault( &m_ensemble, "Ensemble", "Ensemble" );
    m_ensemble.uiCapability()->setUiTreeChildrenHidden( true );
    m_ensemble.uiCapability()->setAutoAddingOptionFromValue( false );

    CAF_PDM_InitFieldNoDefault( &m_ensembleName, "NameAndUnit", "NameAndUnit" );
    m_ensembleName.registerGetMethod( this, &RimWellRftEnsembleCurveSet::ensembleName );
    m_ensembleName.uiCapability()->setUiHidden( true );
    m_ensembleName.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_eclipseCase, "EclipseResultCase", "Eclipse Result Case" );
    m_eclipseCase.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_appearance, "Appearance", "Appearance" );
    m_appearance = new RimCurveSetAppearance();
    m_appearance.uiCapability()->setUiTreeHidden( true );
    m_appearance->appearanceChanged.connect( this, &RimWellRftEnsembleCurveSet::updatePlot );

    CAF_PDM_InitField( &m_ensembleColorMode_OBSOLETE,
                       "ColorMode",
                       RimEnsembleCurveSetColorManager::ColorModeEnum( RimEnsembleCurveSetColorManager::ColorMode::SINGLE_COLOR ),
                       "Coloring Mode" );
    m_ensembleColorMode_OBSOLETE.uiCapability()->setUiHidden( true );
    m_ensembleColorMode_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitField( &m_ensembleParameter_OBSOLETE, "EnsembleParameter", QString( "" ), "Ensemble Parameter" );
    m_ensembleParameter_OBSOLETE.uiCapability()->setUiHidden( true );
    m_ensembleParameter_OBSOLETE.xmlCapability()->setIOWritable( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellRftEnsembleCurveSet::~RimWellRftEnsembleCurveSet()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryEnsemble* RimWellRftEnsembleCurveSet::ensemble() const
{
    return m_ensemble;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftEnsembleCurveSet::setEnsemble( RimSummaryEnsemble* ensemble )
{
    m_ensemble = ensemble;

    m_appearance->setEnsemble( ensemble );

    clearEnsembleStatistics();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftEnsembleCurveSet::setCurveColor( const cvf::Color3f& color )
{
    m_appearance->setMainEnsembleColor( color );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimWellRftEnsembleCurveSet::curveColor( RimSummaryEnsemble* ensemble, const RimSummaryCase* summaryCase ) const
{
    if ( !summaryCase ) return m_appearance->statisticsCurveColor();

    return m_appearance->curveColor( ensemble, summaryCase );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftEnsembleCurveSet::updatePlot( const SignalEmitter* emitter )
{
    if ( RimWellRftPlot* rftPlot = firstAncestorOrThisOfType<RimWellRftPlot>() )
    {
        rftPlot->rebuildCurves();
    }

    // Required to update the color legend object, as this object is only present in the Project Tree when ensemble parameter is used
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftEnsembleCurveSet::clearEnsembleStatistics()
{
    m_statisticsEclipseRftReader = std::make_unique<RifReaderEnsembleStatisticsRft>( m_ensemble(), m_eclipseCase() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftEnsembleCurveSet::initAfterRead()
{
    if ( RimProject::current()->isProjectFileVersionEqualOrOlderThan( "2025.04" ) )
    {
        m_appearance->setColorMode( m_ensembleColorMode_OBSOLETE() );
        m_appearance->setEnsembleParameter( m_ensembleParameter_OBSOLETE() );
    }

    clearEnsembleStatistics();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig* RimWellRftEnsembleCurveSet::legendConfig()
{
    return m_appearance->legendConfig();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftEnsembleCurveSet::setEclipseCase( RimEclipseCase* eclipseCase )
{
    m_eclipseCase = eclipseCase;
    clearEnsembleStatistics();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimWellRftEnsembleCurveSet::eclipseCase() const
{
    return m_eclipseCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderRftInterface* RimWellRftEnsembleCurveSet::statisticsEclipseRftReader()
{
    return m_statisticsEclipseRftReader.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftEnsembleCurveSet::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_eclipseCase )
    {
        clearEnsembleStatistics();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellRftEnsembleCurveSet::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &m_eclipseCase )
    {
        RimTools::caseOptionItems( &options );

        options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftEnsembleCurveSet::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_eclipseCase );

    auto group = uiOrdering.addNewGroup( "Appearance" );
    m_appearance->uiOrdering( uiConfigName, *group );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftEnsembleCurveSet::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= "" */ )
{
    if ( auto legendConfig = m_appearance->legendConfig() )
    {
        uiTreeOrdering.add( legendConfig );
    }
    uiTreeOrdering.skipRemainingChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellRftEnsembleCurveSet::userDescriptionField()
{
    return &m_ensembleName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellRftEnsembleCurveSet::ensembleName() const
{
    if ( m_ensemble ) return m_ensemble->name();

    return "";
}
