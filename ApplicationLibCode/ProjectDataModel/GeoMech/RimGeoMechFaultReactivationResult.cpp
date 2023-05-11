/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-     Equinor ASA
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

#include "RimGeoMechFaultReactivationResult.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RicWellLogTools.h"
#include "WellLogCommands/RicNewWellLogPlotFeatureImpl.h"
#include "WellLogCommands/RicWellLogPlotCurveFeatureImpl.h"

#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"
#include "RimGridView.h"
#include "RimIntersectionCollection.h"
#include "RimModeledWellPath.h"
#include "RimWellLogDiffCurve.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogTrack.h"
#include "RimWellPathGeometryDef.h"

#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPathCollection.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiPushButtonEditor.h"

CAF_PDM_SOURCE_INIT( RimGeoMechFaultReactivationResult, "RimGeoMechFaultReactivationResult" );

#pragma optimize( "", off )

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechFaultReactivationResult::RimGeoMechFaultReactivationResult()
{
    // TODO: Update icon
    CAF_PDM_InitObject( "Fault Reactivation Result", ":/GeoMechCase24x24.png" );

    CAF_PDM_InitFieldNoDefault( &m_intersection, "Intersection", "Intersection" );

    CAF_PDM_InitField( &m_distanceFromIntersection, "FaceDistanceFromIntersection", 0.0, "Face Distance From Intersection" );
    CAF_PDM_InitField( &m_widthOutsideIntersection, "FaceWidthOutsideIntersection", 0.0, "Face Width Outside Intersection" );

    CAF_PDM_InitFieldNoDefault( &m_createFaultReactivationResult, "CreateReactivationResult", "" );
    caf::PdmUiPushButtonEditor::configureEditorForField( &m_createFaultReactivationResult );

    CAF_PDM_InitFieldNoDefault( &m_faceAWellPath, "FaceAWellPath", "Face A Well Path" );
    m_faceAWellPath.uiCapability()->setUiHidden( true );
    CAF_PDM_InitFieldNoDefault( &m_faceBWellPath, "FaceBWellPath", "Face B Well Path" );
    m_faceBWellPath.uiCapability()->setUiHidden( true );

    setDeletable( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechFaultReactivationResult::~RimGeoMechFaultReactivationResult()
{
    delete m_faceAWellPath;
    delete m_faceBWellPath;
}

void RimGeoMechFaultReactivationResult::onLoadDataAndUpdate()
{
    createWellGeometry();
    createWellLogCurves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimGeoMechFaultReactivationResult::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_intersection )
    {
        RimGridView* activeView = RiaApplication::instance()->activeGridView();
        if ( !activeView || !activeView->intersectionCollection() ) return options;

        for ( auto* intersection : activeView->intersectionCollection()->intersections() )
        {
            // Only utilize polyline intersections?
            if ( intersection && intersection->type() == RimExtrudedCurveIntersection::CrossSectionEnum::CS_POLYLINE )
                options.push_back( caf::PdmOptionItemInfo( intersection->name(), intersection ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechFaultReactivationResult::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Fault Reactivation Result" );
    group->add( &m_intersection );
    group->add( &m_distanceFromIntersection );
    group->add( &m_widthOutsideIntersection );
    group->add( &m_createFaultReactivationResult );
    group->add( &m_faceAWellPath );
    group->add( &m_faceBWellPath );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechFaultReactivationResult::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                          const QVariant&            oldValue,
                                                          const QVariant&            newValue )
{
    if ( changedField == &m_distanceFromIntersection || changedField == &m_widthOutsideIntersection )
    {
        createWellGeometry();
    }
    if ( changedField == &m_createFaultReactivationResult && m_intersection() )
    {
        onLoadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechFaultReactivationResult::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                               QString                    uiConfigName,
                                                               caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_createFaultReactivationResult )
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( attrib )
        {
            attrib->m_buttonText = "Create";
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechFaultReactivationResult::createWellGeometry()
{
    RimWellPathCollection* wellPathCollection = RimProject::current()->activeOilField()->wellPathCollection();
    if ( !wellPathCollection ) return;

    // Create well paths if not existing collection
    const auto allWellPaths = wellPathCollection->allWellPaths();
    if ( !m_faceAWellPath ||
         ( m_faceAWellPath && std::find( allWellPaths.begin(), allWellPaths.end(), m_faceAWellPath ) == allWellPaths.end() ) )
    {
        m_faceAWellPath = new RimModeledWellPath();
        m_faceAWellPath->setName( "Fault Face A Well" );
        wellPathCollection->addWellPath( m_faceAWellPath );
    }
    if ( !m_faceBWellPath ||
         ( m_faceBWellPath && std::find( allWellPaths.begin(), allWellPaths.end(), m_faceBWellPath ) == allWellPaths.end() ) )
    {
        m_faceBWellPath = new RimModeledWellPath();
        m_faceBWellPath->setName( "Fault Face B Well" );
        wellPathCollection->addWellPath( m_faceBWellPath );
    }

    if ( !m_faceAWellPath->geometryDefinition() || !m_faceBWellPath->geometryDefinition() ) return;

    // Delete the previous well path target values
    m_faceAWellPath->geometryDefinition()->deleteAllTargets();
    m_faceBWellPath->geometryDefinition()->deleteAllTargets();

    // Using first two points from first polyline
    const auto polyLines = m_intersection()->polyLines();
    if ( polyLines.size() != 1 || polyLines[0].size() != 2 )
    {
        RiaLogging::error( "Polyline intersection for fault face must be defined with only 2 points!" );
        return;
    }
    const std::vector<cvf::Vec3d> wellPoints = { polyLines[0][0], polyLines[0][1] };

    // TODO:
    // - Check which cell point is in, then check which part this cell inn -> provide part as name for curves
    //
    // NOTES:
    // - No data for face B well path when creating well log extraction curves? Empty data in part 2-1?

    // Create vector for well path defined by point a and b
    const cvf::Vec3d a          = wellPoints[0];
    const cvf::Vec3d b          = wellPoints[1];
    const cvf::Vec3d wellVector = b - a;

    // Cross product off well path vector and z-axis. New vector must be normalized
    const cvf::Vec3d normVector     = wellVector ^ cvf::Vector3<double>::Z_AXIS;
    const cvf::Vec3d distanceVector = m_distanceFromIntersection() * normVector.getNormalized();

    // Get normalized vector along well to adjust point a and b outside of defined intersection
    const auto       normalizedWellVector = wellVector.getNormalized();
    const cvf::Vec3d widthAdjustedA       = a - ( normalizedWellVector * m_widthOutsideIntersection() );
    const cvf::Vec3d widthAdjustedB       = b + ( normalizedWellVector * m_widthOutsideIntersection() );

    const std::vector<cvf::Vec3d> newFaceAWellPoints = { widthAdjustedA + distanceVector, widthAdjustedB + distanceVector };
    // const std::vector<cvf::Vec3d> newFaceBWellPoints = { widthAdjustedA - distanceVector, widthAdjustedB - distanceVector };
    const std::vector<cvf::Vec3d> newFaceBWellPoints = { widthAdjustedA + 2.0 * distanceVector, widthAdjustedB + 2.0 * distanceVector };

    // Update the well paths
    m_faceAWellPath->geometryDefinition()->createAndInsertTargets( newFaceAWellPoints );
    m_faceBWellPath->geometryDefinition()->createAndInsertTargets( newFaceBWellPoints );
    m_faceAWellPath->geometryDefinition()->setUseAutoGeneratedTargetAtSeaLevel( false );
    m_faceBWellPath->geometryDefinition()->setUseAutoGeneratedTargetAtSeaLevel( false );
    m_faceAWellPath->createWellPathGeometry();
    m_faceBWellPath->createWellPathGeometry();

    // Update UI
    wellPathCollection->uiCapability()->updateConnectedEditors();
    RimProject::current()->scheduleCreateDisplayModelAndRedrawAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechFaultReactivationResult::createWellLogCurves()
{
    // Create well log extraction curve for m_faceAWellPath and m_faceBWellPath
    // Create well log diff curve for the two well log extraction curves

    // NOTE:
    // - Single track with 3 curves or 3 tracks with 1 curve each

    // Create well log extraction curves
    RimWellLogTrack* faultReactivationTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack();
    faultReactivationTrack->setDescription( "Fault Reactivation Curves" );

    RimGeoMechCase* geomCase = nullptr;
    firstAncestorOrThisOfTypeAsserted( geomCase );
    if ( !geomCase ) return;

    Rim3dView* view = RiaApplication::instance()->activeMainOrComparisonGridView();
    if ( !view ) return;

    RigFemResultAddress        wellLogExtractionResult( RigFemResultPosEnum::RIG_NODAL, "U", "U_LENGTH" );
    RimWellLogExtractionCurve* faceAWellLogExtractionCurve =
        RicWellLogTools::addWellLogExtractionCurve( faultReactivationTrack, geomCase, view, m_faceAWellPath(), nullptr, -1, true );
    RimWellLogExtractionCurve* faceBWellLogExtractionCurve =
        RicWellLogTools::addWellLogExtractionCurve( faultReactivationTrack, geomCase, view, m_faceBWellPath(), nullptr, -1, true );
    faceAWellLogExtractionCurve->setGeoMechResultAddress( wellLogExtractionResult );
    faceBWellLogExtractionCurve->setGeoMechResultAddress( wellLogExtractionResult );
    faceAWellLogExtractionCurve->updateConnectedEditors();
    faceBWellLogExtractionCurve->updateConnectedEditors();

    // Create well log diff curve for m_faceAWellPath and m_faceBWellPath
    // RicWellLogTools::addWellLogDiffCurve( faultReactivationTrack, geomCase, view );
    RimWellLogDiffCurve* faceWellLogDiffCurve = new RimWellLogDiffCurve();
    faceWellLogDiffCurve->setWellLogCurves( faceAWellLogExtractionCurve, faceBWellLogExtractionCurve );
    const cvf::Color3f curveColor = RicWellLogPlotCurveFeatureImpl::curveColorFromTable( faultReactivationTrack->curveCount() );
    faceWellLogDiffCurve->setColor( curveColor );

    faultReactivationTrack->addCurve( faceWellLogDiffCurve );

    faceWellLogDiffCurve->loadDataAndUpdate( true );
    faceWellLogDiffCurve->updateConnectedEditors();
    faultReactivationTrack->updateConnectedEditors();
}
