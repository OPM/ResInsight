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

#include "RimModeledWellPath.h"

#include "RicfCommandObject.h"
#include "RimProject.h"
#include "RimWellPathGeometryDef.h"

#include "RigWellPath.h"

#include "RiaCompletionTypeCalculationScheduler.h"
#include "RifTextDataTableFormatter.h"
#include "RimExtrudedCurveIntersection.h"
#include "RimPlotCurve.h"
#include "RimWellPath.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"
#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT( RimModeledWellPath, "ModeledWellPath" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimModeledWellPath::RimModeledWellPath()
{
    RICF_InitObject( "Modeled WellPath", ":/EditableWell.png", "", "A Well Path created interactively in ResInsight" );

    CAF_PDM_InitFieldNoDefault( &m_geometryDefinition, "WellPathGeometryDef", "Trajectory", "", "", "" );
    m_geometryDefinition = new RimWellPathGeometryDef;

    // Required, as these settings are set in RimWellPath()
    m_name.uiCapability()->setUiReadOnly( false );
    m_name.xmlCapability()->setIOReadable( true );
    m_name.xmlCapability()->setIOWritable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimModeledWellPath::~RimModeledWellPath()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimModeledWellPath::createWellPathGeometry()
{
    this->setWellPathGeometry( m_geometryDefinition->createWellPathGeometry().p() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimModeledWellPath::updateWellPathVisualization()
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
void RimModeledWellPath::scheduleUpdateOfDependentVisualization()
{
    RiaCompletionTypeCalculationScheduler::instance()->scheduleRecalculateCompletionTypeAndRedrawAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathGeometryDef* RimModeledWellPath::geometryDefinition() const
{
    return m_geometryDefinition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimModeledWellPath::wellPlanText()
{
    QString     planText;
    QTextStream qtxtStream( &planText );

    RifTextDataTableFormatter formatter( qtxtStream );
    formatter.setUnlimitedDataRowWidth();
    formatter.setTableRowPrependText( "" );
    formatter.setTableRowLineAppendText( "" );

    std::vector<RifTextDataTableColumn> tableHeader;
    tableHeader.push_back( {"MDRKB"} );
    tableHeader.push_back( {"CL"} );
    tableHeader.push_back( {"Inc"} );
    tableHeader.push_back( {"Azi"} );
    tableHeader.push_back( {"TVDMSL"} );
    tableHeader.push_back( {"NS"} );
    tableHeader.push_back( {"EW"} );
    tableHeader.push_back( {"Dogleg"} );
    tableHeader.push_back( {"Build"} );
    tableHeader.push_back( {"Turn"} );
    formatter.header( tableHeader );

    double mdrkbAtFirstTarget = m_geometryDefinition->mdrkbAtFirstTarget();
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
void RimModeledWellPath::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    uiTreeOrdering.add( m_geometryDefinition() );
    RimWellPath::defineUiTreeOrdering( uiTreeOrdering, uiConfigName );
}
