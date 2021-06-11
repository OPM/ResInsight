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

#include "RiaCompletionTypeCalculationScheduler.h"
#include "RicfCommandObject.h"
#include "RifTextDataTableFormatter.h"
#include "RigWellPath.h"

#include "RimExtrudedCurveIntersection.h"
#include "RimPlotCurve.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RimWellPath.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"
#include "RimWellPathGeometryDef.h"
#include "RimWellPathTarget.h"
#include "RimWellPathTieIn.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmUiDoubleValueEditor.h"
#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT( RimModeledWellPath, "ModeledWellPath" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimModeledWellPath::RimModeledWellPath()
{
    CAF_PDM_InitScriptableObject( "Modeled Well Path",
                                  ":/EditableWell.png",
                                  "",
                                  "A Well Path created interactively in ResInsight" );

    CAF_PDM_InitScriptableFieldWithScriptKeywordNoDefault( &m_geometryDefinition,
                                                           "WellPathGeometryDef",
                                                           "WellPathGeometry",
                                                           "Trajectory",
                                                           "",
                                                           "",
                                                           "" );
    m_geometryDefinition = new RimWellPathGeometryDef;
    m_geometryDefinition->changed.connect( this, &RimModeledWellPath::onGeometryDefinitionChanged );

    // Required, as these settings are set in RimWellPath()
    m_name.uiCapability()->setUiReadOnly( false );
    m_name.uiCapability()->setUiHidden( false );
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
    std::vector<QString> columns = { "MDRKB", "CL", "Inc", "Azi", "TVDMSL", "NS", "EW", "Dogleg", "Build", "Turn" };
    for ( QString column : columns )
    {
        tableHeader.push_back(
            RifTextDataTableColumn( column,
                                    RifTextDataTableDoubleFormatting( RifTextDataTableDoubleFormat::RIF_FLOAT, 2 ) ) );
    }

    formatter.header( tableHeader );

    double mdrkbAtFirstTarget = m_geometryDefinition->mdAtFirstTarget() + m_geometryDefinition->airGap();
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimModeledWellPath::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_name );

    RimWellPath::defineUiOrdering( uiConfigName, uiOrdering );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimModeledWellPath::onGeometryDefinitionChanged( const caf::SignalEmitter* emitter, bool fullUpdate )
{
    updateGeometry( fullUpdate );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimModeledWellPath::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                           const QVariant&            oldValue,
                                           const QVariant&            newValue )
{
    // TODO remove if nothing happens here

    RimWellPath::fieldChangedByUi( changedField, oldValue, newValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimModeledWellPath::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                         bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimModeledWellPath::updateGeometry( bool fullUpdate )
{
    updateWellPathVisualization();

    std::vector<RimWellPathTieIn*> tieInObjects;
    objectsWithReferringPtrFieldsOfType( tieInObjects );
    for ( auto tieIn : tieInObjects )
    {
        if ( tieIn->parentWell() == this )
        {
            tieIn->updateChildWellGeometry();
        }
    }

    if ( fullUpdate )
    {
        scheduleUpdateOfDependentVisualization();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimModeledWellPath::updateTieInLocationFromParentWell()
{
    RimWellPath* parentWellPath = nullptr;

    RimWellPathTieIn* tieIn = wellPathTieIn();
    if ( tieIn )
    {
        parentWellPath = tieIn->parentWell();

        auto targets = m_geometryDefinition->activeWellTargets();
        if ( parentWellPath && !targets.empty() )
        {
            auto [pointVector, measuredDepths] =
                parentWellPath->wellPathGeometry()
                    ->clippedPointSubset( parentWellPath->wellPathGeometry()->measuredDepths().front(),
                                          tieIn->tieInMeasuredDepth() );

            if ( pointVector.size() > 2u )
            {
                auto firstTarget = targets.front();
                firstTarget->setPointXYZ( pointVector.back() );

                m_geometryDefinition->setIsAttachedToParentWell( true );
                m_geometryDefinition->setMdAtFirstTarget( measuredDepths.back() );
                m_geometryDefinition->setFixedWellPathPoints( pointVector );
                m_geometryDefinition->setFixedMeasuredDepths( measuredDepths );

                updateGeometry( true );
            }
        }
    }

    if ( !parentWellPath )
    {
        m_geometryDefinition->setIsAttachedToParentWell( false );
        m_geometryDefinition->setFixedWellPathPoints( {} );
        m_geometryDefinition->setFixedMeasuredDepths( {} );
    }
}
