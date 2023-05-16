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

#include "RigFemPartCollection.h"
#include "RigGeoMechCaseData.h"
#include "RigReservoirGridTools.h"

#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"
#include "RimGridView.h"
#include "RimIntersectionCollection.h"
#include "RimModeledWellPath.h"
#include "RimWellLogDiffCurve.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogPlotNameConfig.h"
#include "RimWellLogTrack.h"
#include "RimWellPathGeometryDef.h"

#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPathCollection.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiPushButtonEditor.h"

#include "cvfBoundingBox.h"

CAF_PDM_SOURCE_INIT( RimGeoMechFaultReactivationResult, "RimGeoMechFaultReactivationResult" );

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

    CAF_PDM_InitField( &m_faceAWellPathPartIndex, "FaceAWellPathPartIndex", 0, "Face A Well Path Part Index" );
    m_faceAWellPathPartIndex.uiCapability()->setUiHidden( true );
    CAF_PDM_InitField( &m_faceBWellPathPartIndex, "FaceBWellPathPartIndex", 0, "Face B Well Path Part Index" );
    m_faceBWellPathPartIndex.uiCapability()->setUiHidden( true );

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
            // Only utilize polyline intersections with two points
            if ( intersection && intersection->type() == RimExtrudedCurveIntersection::CrossSectionEnum::CS_POLYLINE &&
                 !intersection->polyLines().empty() && intersection->polyLines()[0].size() == 2 )
            {
                options.push_back( caf::PdmOptionItemInfo( intersection->name(), intersection ) );
            }
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
    group->add( &m_faceAWellPathPartIndex );
    group->add( &m_faceBWellPathPartIndex );
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
    RimGeoMechCase* geomCase = firstAncestorOrThisOfTypeAsserted<RimGeoMechCase>();
    if ( !geomCase || !geomCase->geoMechData() ) return;
    RigFemPartCollection* geoMechPartCollection = geomCase->geoMechData()->femParts();
    if ( !geoMechPartCollection ) return;
    RimWellPathCollection* wellPathCollection = RimProject::current()->activeOilField()->wellPathCollection();
    if ( !wellPathCollection ) return;

    // Create well paths if not existing collection
    const auto allWellPaths = wellPathCollection->allWellPaths();
    if ( !m_faceAWellPath ||
         ( m_faceAWellPath && std::find( allWellPaths.begin(), allWellPaths.end(), m_faceAWellPath ) == allWellPaths.end() ) )
    {
        m_faceAWellPath = new RimModeledWellPath();
        m_faceAWellPath->setName( "Fault Face A Well" );
        m_faceAWellPath->setShowWellPath( false );
        wellPathCollection->addWellPath( m_faceAWellPath );
    }
    if ( !m_faceBWellPath ||
         ( m_faceBWellPath && std::find( allWellPaths.begin(), allWellPaths.end(), m_faceBWellPath ) == allWellPaths.end() ) )
    {
        m_faceBWellPath = new RimModeledWellPath();
        m_faceBWellPath->setName( "Fault Face B Well" );
        m_faceBWellPath->setShowWellPath( false );
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
    const std::vector<cvf::Vec3d> points = { polyLines[0][0], polyLines[0][1] };

    // Create vector for well path defined by point a and b
    const cvf::Vec3d a          = points[0];
    const cvf::Vec3d b          = points[1];
    const cvf::Vec3d wellVector = b - a;

    // Cross product off well path vector and z-axis (New vector must be normalized)
    const cvf::Vec3d normVector     = wellVector ^ cvf::Vector3<double>::Z_AXIS;
    const cvf::Vec3d distanceVector = m_distanceFromIntersection() * normVector.getNormalized();

    // Get normalized vector along well to adjust point a and b outside of defined intersection
    const auto       normalizedWellVector = wellVector.getNormalized();
    const cvf::Vec3d widthAdjustedA       = a - ( normalizedWellVector * m_widthOutsideIntersection() );
    const cvf::Vec3d widthAdjustedB       = b + ( normalizedWellVector * m_widthOutsideIntersection() );

    // Create well points for face A and B
    const std::pair<cvf::Vec3d, cvf::Vec3d> faceAWellStartAndEnd = { widthAdjustedA + distanceVector, widthAdjustedB + distanceVector };
    const std::pair<cvf::Vec3d, cvf::Vec3d> faceBWellStartAndEnd = { widthAdjustedA - distanceVector, widthAdjustedB - distanceVector };

    // Get center point between face well points to detect which part fault faces are in
    auto             centerpoint         = []( const cvf::Vec3d& a, const cvf::Vec3d& b ) -> cvf::Vec3d { return ( a + b ) / 2.0; };
    const cvf::Vec3d faceAWellPathCenter = centerpoint( faceAWellStartAndEnd.first, faceAWellStartAndEnd.second );
    const cvf::Vec3d faceBWellPathCenter = centerpoint( faceBWellStartAndEnd.first, faceBWellStartAndEnd.second );

    // Update the well path target values
    const std::vector<cvf::Vec3d> faceAWellPoints = { faceAWellStartAndEnd.first, faceAWellPathCenter, faceAWellStartAndEnd.second };
    const std::vector<cvf::Vec3d> faceBWellPoints = { faceBWellStartAndEnd.first, faceBWellPathCenter, faceBWellStartAndEnd.second };
    m_faceAWellPath->geometryDefinition()->createAndInsertTargets( faceAWellPoints );
    m_faceBWellPath->geometryDefinition()->createAndInsertTargets( faceBWellPoints );
    m_faceAWellPath->geometryDefinition()->setUseAutoGeneratedTargetAtSeaLevel( false );
    m_faceBWellPath->geometryDefinition()->setUseAutoGeneratedTargetAtSeaLevel( false );
    m_faceAWellPath->createWellPathGeometry();
    m_faceBWellPath->createWellPathGeometry();

    // Detect which part well path centers are in
    m_faceAWellPathPartIndex = getPartIndexFromPoint( geoMechPartCollection, faceAWellPathCenter );
    m_faceBWellPathPartIndex = getPartIndexFromPoint( geoMechPartCollection, faceBWellPathCenter );

    // Update UI
    wellPathCollection->uiCapability()->updateConnectedEditors();
    RimProject::current()->scheduleCreateDisplayModelAndRedrawAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechFaultReactivationResult::createWellLogCurves()
{
    RimGeoMechCase* geomCase = firstAncestorOrThisOfTypeAsserted<RimGeoMechCase>();
    if ( !geomCase ) return;
    Rim3dView* view = RiaApplication::instance()->activeMainOrComparisonGridView();
    if ( !view ) return;

    // Create Plot
    const bool      showAfterCreation = true;
    RimWellLogPlot* newPlot = RicNewWellLogPlotFeatureImpl::createWellLogPlot( showAfterCreation, QString( "Fault Reactivation Plot" ) );
    newPlot->setNamingMethod( RiaDefines::ObjectNamingMethod::CUSTOM );
    newPlot->nameConfig()->setCustomName( "Fault Reactivation Plot" );

    // Create curve tracks
    const bool       doUpdateAfter = true;
    RimWellLogTrack* wellLogExtractionDisplacementTrack =
        RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack( doUpdateAfter, QString( "Fault Reactivation Displacement Curves" ), newPlot );
    RimWellLogTrack* wellLogDiffTrack =
        RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack( doUpdateAfter, QString( "Fault Reactivation Displacement Diff" ), newPlot );
    RimWellLogTrack* wellLogExtractionFaultmobTrack =
        RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack( doUpdateAfter, QString( "Fault Reactivation Faultmob Curves" ), newPlot );

    // Well log extraction displacement curves
    RigFemResultAddress wellLogExtractionDisplacementResult( RigFemResultPosEnum::RIG_NODAL, "U", "U_LENGTH" );
    auto*               faceADisplacementCurve = createWellLogExtractionCurveAndAddToTrack( wellLogExtractionDisplacementTrack,
                                                                              wellLogExtractionDisplacementResult,
                                                                              m_faceAWellPath(),
                                                                              m_faceAWellPathPartIndex() );
    auto*               faceBDisplacementCurve = createWellLogExtractionCurveAndAddToTrack( wellLogExtractionDisplacementTrack,
                                                                              wellLogExtractionDisplacementResult,
                                                                              m_faceBWellPath(),
                                                                              m_faceBWellPathPartIndex() );

    if ( !faceADisplacementCurve || !faceBDisplacementCurve )
    {
        RiaLogging::error( "Failed to create well log extraction displacement curves" );
        return;
    }

    // Create well log diff curve for m_faceAWellPath and m_faceBWellPath
    RimWellLogDiffCurve* faceWellLogDiffCurve = RicWellLogTools::addWellLogDiffCurve( wellLogDiffTrack );
    faceWellLogDiffCurve->setWellLogCurves( faceADisplacementCurve, faceBDisplacementCurve );
    faceWellLogDiffCurve->loadDataAndUpdate( true );
    faceWellLogDiffCurve->updateConnectedEditors();

    // Well log extraction faultmob curves
    RigFemResultAddress wellLogExtractionFaultmobResult( RigFemResultPosEnum::RIG_ELEMENT_NODAL_FACE, "SE", "FAULTMOB" );
    createWellLogExtractionCurveAndAddToTrack( wellLogExtractionFaultmobTrack,
                                               wellLogExtractionFaultmobResult,
                                               m_faceAWellPath(),
                                               m_faceAWellPathPartIndex() );
    createWellLogExtractionCurveAndAddToTrack( wellLogExtractionFaultmobTrack,
                                               wellLogExtractionFaultmobResult,
                                               m_faceBWellPath(),
                                               m_faceBWellPathPartIndex() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimGeoMechFaultReactivationResult::getPartIndexFromPoint( const RigFemPartCollection* const partCollection, const cvf::Vec3d& point ) const
{
    int idx = 0;
    if ( !partCollection ) return idx;

    const cvf::BoundingBox intersectingBb( point, point );
    std::vector<size_t>    intersectedGlobalElementIndices;
    partCollection->findIntersectingGlobalElementIndices( intersectingBb, &intersectedGlobalElementIndices );

    if ( intersectedGlobalElementIndices.empty() ) return idx;

    // Utilize first intersected element to detect part for point
    const auto [partId, elementIndex] = partCollection->partIdAndElementIndex( intersectedGlobalElementIndices.front() );
    idx                               = partId;

    return idx;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogExtractionCurve* RimGeoMechFaultReactivationResult::createWellLogExtractionCurveAndAddToTrack( RimWellLogTrack* track,
                                                                                                         const RigFemResultAddress& resultAddress,
                                                                                                         RimModeledWellPath* wellPath,
                                                                                                         int                 partId )
{
    RimGeoMechCase* geomCase = firstAncestorOrThisOfTypeAsserted<RimGeoMechCase>();
    if ( !geomCase ) return nullptr;
    Rim3dView* view = RiaApplication::instance()->activeMainOrComparisonGridView();
    if ( !view ) return nullptr;

    const int  branchIndex        = -1;
    const bool useBranchDetection = true;
    const bool updateParentPlot   = true;

    RimWellLogExtractionCurve* wellLogExtractionCurve =
        RicWellLogTools::addWellLogExtractionCurve( track, geomCase, view, wellPath, nullptr, branchIndex, useBranchDetection );
    wellLogExtractionCurve->setGeoMechResultAddress( resultAddress );
    wellLogExtractionCurve->setGeoMechPart( partId );
    wellLogExtractionCurve->updateConnectedEditors();
    wellLogExtractionCurve->loadDataAndUpdate( updateParentPlot );

    return wellLogExtractionCurve;
}
