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

#include "RimWellBoreStabilityPlot.h"

#include "RiaDefines.h"
#include "RicfCommandObject.h"

#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigGeoMechCaseData.h"

#include "RimGeoMechCase.h"
#include "RimTools.h"
#include "RimWbsParameters.h"
#include "RimWellLogCurveCommonDataSource.h"
#include "RimWellLogLasFile.h"
#include "RimWellLogPlotNameConfig.h"

#include "cafPdmBase.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObject.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiGroup.h"

CAF_PDM_SOURCE_INIT( RimWellBoreStabilityPlot, "WellBoreStabilityPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellBoreStabilityPlot::RimWellBoreStabilityPlot()
{
    CAF_PDM_InitScriptableObject( "Well Bore Stability Plot", ":/WellBoreStability16x16.png", "", "A GeoMechanical Well Bore Stability Plot" );

    CAF_PDM_InitScriptableFieldWithScriptKeywordNoDefault( &m_wbsParameters, "WbsParameters", "Parameters", "Well Bore Stability Parameters" );
    m_wbsParameters = new RimWbsParameters;
    m_wbsParameters.uiCapability()->setUiTreeChildrenHidden( true );

    m_nameConfig->setCustomName( "Well Bore Stability" );
    m_nameConfig->enableAllAutoNameTags( true );

    m_commonDataSource->setCaseType( RiaDefines::CaseType::GEOMECH_ODB_CASE );

    m_waterDepth = std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellBoreStabilityPlot::applyWbsParametersToExtractor( RigGeoMechWellLogExtractor* extractor )
{
    auto originalValue = m_waterDepth;
    m_waterDepth       = extractor->waterDepth();

    if ( m_waterDepth == std::numeric_limits<double>::infinity() )
    {
        m_waterDepth = extractor->estimateWaterDepth();
    }

    m_wbsParameters->applyWbsParametersToExtractor( extractor );

    if ( originalValue != m_waterDepth )
    {
        performAutoNameUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellBoreStabilityPlot::userDefinedValue( const RigWbsParameter& parameter ) const
{
    return m_wbsParameters->userDefinedValue( parameter );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellBoreStabilityPlot::copyWbsParameters( const RimWbsParameters* wbsParameters )
{
    if ( wbsParameters )
    {
        *m_wbsParameters = *wbsParameters;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellBoreStabilityPlot::setCaseWellPathAndTimeStep( RimGeoMechCase* geoMechCase, RimWellPath* wellPath, int timeStep, int frameIndex /* = -1 */ )
{
    m_wbsParameters->setGeoMechCase( geoMechCase );
    m_wbsParameters->setWellPath( wellPath );
    m_wbsParameters->setTimeStep( timeStep );
    m_wbsParameters->setFrameIndex( frameIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellBoreStabilityPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    m_commonDataSource->uiOrdering( RimWellLogCurveCommonDataSource::smoothingUiOrderinglabel(), uiOrdering );

    caf::PdmUiGroup* parametersGroup = uiOrdering.addNewGroup( "Parameter Sources" );
    m_wbsParameters->uiOrdering( uiConfigName, *parametersGroup );

    caf::PdmUiGroup* depthGroup = uiOrdering.addNewGroup( "Depth Axis" );
    RimWellLogPlot::uiOrderingForDepthAxis( uiConfigName, *depthGroup );

    caf::PdmUiGroup* titleGroup = uiOrdering.addNewGroup( "Plot Title" );
    RimWellLogPlot::uiOrderingForAutoName( uiConfigName, *titleGroup );

    caf::PdmUiGroup* legendGroup = uiOrdering.addNewGroup( "Legends" );
    legendGroup->setCollapsedByDefault();
    RimPlotWindow::uiOrderingForLegends( uiConfigName, *legendGroup, true );

    RimDepthTrackPlot::uiOrderingForFonts( uiConfigName, uiOrdering );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellBoreStabilityPlot::onLoadDataAndUpdate()
{
    m_wbsParameters->loadDataAndUpdate();
    RimWellLogPlot::onLoadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellBoreStabilityPlot::childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField )
{
    if ( changedChildField == &m_commonDataSource )
    {
        applyDataSource();
    }
    else if ( changedChildField == &m_wbsParameters )
    {
        loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellBoreStabilityPlot::initAfterRead()
{
    updateCommonDataSource();
    applyDataSource();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimWellBoreStabilityPlot::supportedPlotNameVariables() const
{
    auto variables = RimWellLogPlot::supportedPlotNameVariables();

    variables.append( RiaDefines::namingVariableWaterDepth() );

    return variables;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QString, QString> RimWellBoreStabilityPlot::createNameKeyValueMap() const
{
    auto nameKeyValueMap = RimWellLogPlot::createNameKeyValueMap();

    QString waterDepthString                                = QString( "Water Depth = %1 m" ).arg( m_waterDepth );
    nameKeyValueMap[RiaDefines::namingVariableWaterDepth()] = waterDepthString;

    return nameKeyValueMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellBoreStabilityPlot::applyDataSource()
{
    m_wbsParameters->setGeoMechCase( dynamic_cast<RimGeoMechCase*>( m_commonDataSource->caseToApply() ) );
    m_wbsParameters->setWellPath( m_commonDataSource->wellPathToApply() );
    m_wbsParameters->setTimeStep( m_commonDataSource->timeStepToApply() );
    m_wbsParameters->setFrameIndex( -1 );

    updateConnectedEditors();
}
