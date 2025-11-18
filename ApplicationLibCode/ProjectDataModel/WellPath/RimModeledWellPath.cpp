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

#include "CompletionExportCommands/RicWellPathExportCompletionDataFeatureImpl.h"

#include "Well/RigWellPath.h"
#include "Well/RigWellPathGeometryTools.h"

#include "RimExtrudedCurveIntersection.h"
#include "RimPlotCurve.h"
#include "RimProject.h"
#include "RimSeismicSection.h"
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
    CAF_PDM_InitScriptableObject( "Modeled Well Path", ":/EditableWell.png", "", "A Well Path created interactively in ResInsight" );

    CAF_PDM_InitScriptableFieldWithScriptKeywordNoDefault( &m_geometryDefinition, "WellPathGeometryDef", "WellPathGeometry", "Trajectory" );
    m_geometryDefinition = new RimWellPathGeometryDef;
    m_geometryDefinition->changed.connect( this, &RimModeledWellPath::onGeometryDefinitionChanged );
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
    setWellPathGeometry( m_geometryDefinition->createWellPathGeometry().p() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimModeledWellPath::updateWellPathVisualization()
{
    createWellPathGeometry();

    std::vector<RimPlotCurve*> referringCurves = objectsWithReferringPtrFieldsOfType<RimPlotCurve>();
    for ( auto curve : referringCurves )
    {
        curve->loadDataAndUpdate( false );
    }

    for ( auto fracture : fractureCollection()->activeFractures() )
    {
        fracture->loadDataAndUpdate();
    }

    std::vector<RimExtrudedCurveIntersection*> referringIntersections = objectsWithReferringPtrFieldsOfType<RimExtrudedCurveIntersection>();

    for ( auto intersection : referringIntersections )
    {
        intersection->rebuildGeometryAndScheduleCreateDisplayModel();
    }

    std::vector<RimSeismicSection*> referringSeismic = objectsWithReferringPtrFieldsOfType<RimSeismicSection>();
    for ( auto seisSec : referringSeismic )
    {
        seisSec->updateVisualization();
    }

    RimProject* proj = RimProject::current();
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
    QString     plaintext;
    QTextStream qtxtStream( &plaintext );

    RifTextDataTableFormatter formatter( qtxtStream );
    formatter.setUnlimitedDataRowWidth();
    formatter.setTableRowPrependText( "" );
    formatter.setTableRowLineAppendText( "" );

    std::vector<RifTextDataTableColumn> tableHeader;
    std::vector<QString>                columns = { "MDRKB", "CL", "Inc", "Azi", "TVDMSL", "NS", "EW", "Dogleg", "Build", "Turn" };
    for ( QString column : columns )
    {
        tableHeader.push_back(
            RifTextDataTableColumn( column, RifTextDataTableDoubleFormatting( RifTextDataTableDoubleFormat::RIF_FLOAT, 2 ) ) );
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

    return plaintext;
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
void RimModeledWellPath::onGeometryDefinitionChanged( const caf::SignalEmitter* emitter, bool fullUpdate )
{
    updateGeometry( fullUpdate );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimModeledWellPath::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
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

    std::vector<RimWellPathTieIn*> tieInObjects = objectsWithReferringPtrFieldsOfType<RimWellPathTieIn>();
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
        if ( parentWellPath && !targets.empty() && parentWellPath->wellPathGeometry() &&
             !parentWellPath->wellPathGeometry()->measuredDepths().empty() )
        {
            auto [pointVector, measuredDepths] =
                parentWellPath->wellPathGeometry()->clippedPointSubset( parentWellPath->wellPathGeometry()->measuredDepths().front(),
                                                                        tieIn->tieInMeasuredDepth() );

            if ( pointVector.size() >= 2u )
            {
                m_geometryDefinition->setIsAttachedToParentWell( true );
                m_geometryDefinition->setMdAtFirstTarget( measuredDepths.back() );
                m_geometryDefinition->setFixedWellPathPoints( pointVector );
                m_geometryDefinition->setFixedMeasuredDepths( measuredDepths );

                updateReferencePoint();

                auto lastPointXYZ = pointVector.back();

                cvf::Vec3d referencePointXYZ = m_geometryDefinition->anchorPointXyz();
                cvf::Vec3d relativePointXYZ  = lastPointXYZ - referencePointXYZ;

                auto firstTarget = targets.front();
                const auto [azimuth, inclination] =
                    RigWellPathGeometryTools::calculateAzimuthAndInclinationAtMd( tieIn->tieInMeasuredDepth(),
                                                                                  parentWellPath->wellPathGeometry() );
                firstTarget->setAsPointXYZAndTangentTarget( relativePointXYZ, azimuth, inclination );

                updateGeometry( true );
            }
        }
    }

    if ( !parentWellPath )
    {
        m_geometryDefinition->setIsAttachedToParentWell( false );
        m_geometryDefinition->setFixedWellPathPoints( {} );
        m_geometryDefinition->setFixedMeasuredDepths( {} );

        updateWellPathVisualization();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimModeledWellPath::updateReferencePoint()
{
    if ( !m_geometryDefinition->useReferencePointFromTopLevelWell() ) return;

    RimWellPathTieIn* tieIn = wellPathTieIn();
    if ( !tieIn ) return;

    auto topLevelModelledWell = dynamic_cast<RimModeledWellPath*>( topLevelWellPath() );
    if ( !topLevelModelledWell ) return;

    auto refPoint = topLevelModelledWell->geometryDefinition()->anchorPointXyz();
    m_geometryDefinition->setReferencePointXyz( refPoint );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimModeledWellPath::connectWellPaths( RimWellPath* parentWell, double tieInMeasuredDepth )
{
    RimWellPath::connectWellPaths( parentWell, tieInMeasuredDepth );

    updateTieInLocationFromParentWell();
}
