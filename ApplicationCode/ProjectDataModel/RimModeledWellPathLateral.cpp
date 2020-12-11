/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RimModeledWellPathLateral.h"

#include "RicfCommandObject.h"
#include "RimProject.h"
#include "RimWellPathGroup.h"
#include "RimWellPathLateralGeometryDef.h"

#include "RigWellPath.h"

#include "RiaCompletionTypeCalculationScheduler.h"
#include "RifTextDataTableFormatter.h"
#include "RimExtrudedCurveIntersection.h"
#include "RimPlotCurve.h"
#include "RimWellPath.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT( RimModeledWellPathLateral, "ModeledWellPathLateral" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimModeledWellPathLateral::RimModeledWellPathLateral()
{
    CAF_PDM_InitScriptableObject( "Modeled Well Path Lateral",
                                  ":/EditableWell.png",
                                  "",
                                  "A Well Path Lateral created interactively in ResInsight" );

    CAF_PDM_InitScriptableFieldWithScriptKeywordNoDefault( &m_geometryDefinition,
                                                           "WellPathLateralGeometryDef",
                                                           "WellPathLateralGeometry",
                                                           "Trajectory",
                                                           "",
                                                           "",
                                                           "" );

    m_geometryDefinition = new RimWellPathLateralGeometryDef;
    m_geometryDefinition->changed.connect( this, &RimModeledWellPathLateral::onGeometryDefinitionChanged );

    CAF_PDM_InitFieldNoDefault( &m_lateralName, "LateralName", "Lateral Name", "", "", "" );
    m_lateralName.registerGetMethod( this, &RimModeledWellPathLateral::createName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimModeledWellPathLateral::~RimModeledWellPathLateral()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimModeledWellPathLateral::createWellPathGeometry()
{
    this->setWellPathGeometry( m_geometryDefinition->createWellPathGeometry().p() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimModeledWellPathLateral::updateWellPathVisualization()
{
    this->setWellPathGeometry( m_geometryDefinition->createWellPathGeometry().p() );

    std::vector<RimPlotCurve*> refferingCurves;
    this->objectsWithReferringPtrFieldsOfType( refferingCurves );

    for ( auto curve : refferingCurves )
    {
        curve->loadDataAndUpdate( false );
    }

    for ( auto fracture : this->fractureCollection()->activeFractures() )
    {
        fracture->loadDataAndUpdate();
    }

    std::vector<RimExtrudedCurveIntersection*> refferingIntersections;
    this->objectsWithReferringPtrFieldsOfType( refferingIntersections );

    for ( auto intersection : refferingIntersections )
    {
        intersection->rebuildGeometryAndScheduleCreateDisplayModel();
    }

    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted( proj );
    proj->scheduleCreateDisplayModelAndRedrawAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimModeledWellPathLateral::scheduleUpdateOfDependentVisualization()
{
    RiaCompletionTypeCalculationScheduler::instance()->scheduleRecalculateCompletionTypeAndRedrawAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathLateralGeometryDef* RimModeledWellPathLateral::geometryDefinition() const
{
    return m_geometryDefinition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimModeledWellPathLateral::wellPlanText()
{
    QString     planText;
    QTextStream qtxtStream( &planText );

    RifTextDataTableFormatter formatter( qtxtStream );
    formatter.setUnlimitedDataRowWidth();
    formatter.setTableRowPrependText( "" );
    formatter.setTableRowLineAppendText( "" );

    std::vector<RifTextDataTableColumn> tableHeader;
    std::vector<QString> columns = { "MDRKB", "CL", "Inc", "Azi", "TVDMSL", "NS", "EW", "Dogleg", "Build", "Turn" };
    for ( QString column : columns )
    {
        tableHeader.push_back(
            RifTextDataTableColumn( column,
                                    RifTextDataTableDoubleFormatting( RifTextDataTableDoubleFormat::RIF_FLOAT, 2 ) ) );
    }

    formatter.header( tableHeader );

    double mdrkbAtFirstTarget = m_geometryDefinition->mdAtConnection() + parentGroup()->airGap();
    if ( m_geometryDefinition )
    {
        std::vector<RiaWellPlanCalculator::WellPlanSegment> wellPlan = m_geometryDefinition->wellPlan();
        for ( const auto& segment : wellPlan )
        {
            formatter.add( segment.MD + mdrkbAtFirstTarget );
            formatter.add( segment.CL );
            formatter.add( segment.inc );
            formatter.add( segment.azi );
            formatter.add( segment.TVD );
            formatter.add( segment.NS );
            formatter.add( segment.EW );
            formatter.add( segment.dogleg );
            formatter.add( segment.build );
            formatter.add( segment.turn );
            formatter.rowCompleted();
        }
    }
    formatter.tableCompleted();

    return planText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimWellPathGroup* RimModeledWellPathLateral::parentGroup() const
{
    const RimWellPathGroup* group = nullptr;
    this->firstAncestorOrThisOfTypeAsserted( group );
    return group;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimModeledWellPathLateral::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    uiTreeOrdering.add( m_geometryDefinition() );
    RimWellPath::defineUiTreeOrdering( uiTreeOrdering, uiConfigName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimModeledWellPathLateral::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_name );
    RimWellPath::defineUiOrdering( uiConfigName, uiOrdering );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimModeledWellPathLateral::onGeometryDefinitionChanged( const caf::SignalEmitter* emitter, bool fullUpdate )
{
    updateWellPathVisualization();
    if ( fullUpdate )
    {
        scheduleUpdateOfDependentVisualization();
    }
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimModeledWellPathLateral::userDescriptionField()
{
    return &m_lateralName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimModeledWellPathLateral::createName() const
{
    return QString( "%1 [branch md=%2]" ).arg( parentGroup()->createGroupName() ).arg( m_geometryDefinition->mdAtConnection() );
}
