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

#include "RifJsonEncodeDecode.h"

#include "RigFemPartCollection.h"
#include "RigGeoMechCaseData.h"
#include "RigHexIntersectionTools.h"
#include "RigReservoirGridTools.h"

#include "RimFaultReactivationTools.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"
#include "RimGridView.h"
#include "RimMainPlotCollection.h"
#include "RimModeledWellPath.h"
#include "RimWellLogCalculatedCurve.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogPlotCollection.h"
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

#include <array>

#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QVariant>

CAF_PDM_SOURCE_INIT( RimGeoMechFaultReactivationResult, "RimGeoMechFaultReactivationResult" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechFaultReactivationResult::RimGeoMechFaultReactivationResult()
    : m_bHaveValidData( false )
{
    CAF_PDM_InitObject( "Fault Reactivation Result", ":/GeoMechCase24x24.png" );

    CAF_PDM_InitField( &m_distanceFromFault, "DistanceFromFault", 1.0, "Distance From Fault" );

    CAF_PDM_InitFieldNoDefault( &m_createFaultReactivationPlot, "CreateReactivationPlot", "" );
    caf::PdmUiPushButtonEditor::configureEditorForField( &m_createFaultReactivationPlot );

    CAF_PDM_InitFieldNoDefault( &m_faultNormal, "FaultNormal", "" );
    CAF_PDM_InitFieldNoDefault( &m_faultTopPosition, "FaultTopPosition", "" );
    CAF_PDM_InitFieldNoDefault( &m_faultBottomPosition, "FaultBottomPosition", "" );

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
    // delete m_faceAWellPath;
    // delete m_faceBWellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechFaultReactivationResult::onLoadDataAndUpdate()
{
    auto geomCase = geoMechCase();

    auto      filename = geoMechCase()->gridFileName();
    QFileInfo fi( filename );
    auto      folder   = fi.path();
    auto      basename = fi.baseName();

    QDir workDir( folder );
    auto modelSettingsFilename = workDir.absoluteFilePath( basename + ".settings.json" );

    auto map = ResInsightInternalJson::JsonReader::decodeFile( modelSettingsFilename );
    if ( !map.isEmpty() )
    {
        m_faultNormal         = RimFaultReactivationTools::normalVector( map );
        m_faultTopPosition    = RimFaultReactivationTools::topFaultPosition( map );
        m_faultBottomPosition = RimFaultReactivationTools::bottomFaultPosition( map );

        m_bHaveValidData = true;
    }

    createWellGeometry();
    createWellLogCurves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimGeoMechFaultReactivationResult::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechFaultReactivationResult::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Fault Reactivation Result" );
    group->add( &m_distanceFromFault );
    group->add( &m_createFaultReactivationPlot );
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
    if ( changedField == &m_distanceFromFault )
    {
        createWellGeometry();
    }
    if ( changedField == &m_createFaultReactivationPlot )
    {
        createWellGeometry();
        createWellLogCurves();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechFaultReactivationResult::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                               QString                    uiConfigName,
                                                               caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_createFaultReactivationPlot )
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( attrib )
        {
            attrib->m_buttonText = "Create Plot";
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechFaultReactivationResult::createWellGeometry()
{
    if ( !m_bHaveValidData ) return;

    auto geomCase = geoMechCase();
    if ( !geomCase || !geomCase->geoMechData() ) return;
    RigFemPartCollection* geoMechPartCollection = geomCase->geoMechData()->femParts();
    if ( !geoMechPartCollection ) return;
    RimWellPathCollection* wellPathCollection = RimProject::current()->activeOilField()->wellPathCollection();
    if ( !wellPathCollection ) return;

    // Create well paths if not existing in collection
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

    cvf::Vec3d partATop    = m_faultTopPosition() + m_faultNormal() * m_distanceFromFault;
    cvf::Vec3d partABottom = m_faultBottomPosition() + m_faultNormal() * m_distanceFromFault;
    cvf::Vec3d partBTop    = m_faultTopPosition() - m_faultNormal() * m_distanceFromFault;
    cvf::Vec3d partBBottom = m_faultBottomPosition() - m_faultNormal() * m_distanceFromFault;

    // Update the well path target values
    const std::vector<cvf::Vec3d> faceAWellPoints = { partATop, partABottom };
    const std::vector<cvf::Vec3d> faceBWellPoints = { partBTop, partBBottom };
    m_faceAWellPath->geometryDefinition()->createAndInsertTargets( faceAWellPoints );
    m_faceBWellPath->geometryDefinition()->createAndInsertTargets( faceBWellPoints );
    m_faceAWellPath->geometryDefinition()->setUseAutoGeneratedTargetAtSeaLevel( false );
    m_faceBWellPath->geometryDefinition()->setUseAutoGeneratedTargetAtSeaLevel( false );
    m_faceAWellPath->createWellPathGeometry();
    m_faceBWellPath->createWellPathGeometry();

    // Detect which part well path centers are in
    m_faceAWellPathPartIndex = getPartIndexFromPoint( geoMechPartCollection, partATop );
    m_faceBWellPathPartIndex = getPartIndexFromPoint( geoMechPartCollection, partBTop );

    // Update UI
    wellPathCollection->uiCapability()->updateConnectedEditors();
    RimProject::current()->scheduleCreateDisplayModelAndRedrawAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechFaultReactivationResult::createWellLogCurves()
{
    if ( !m_bHaveValidData ) return;

    auto geomCase = geoMechCase();
    if ( !geomCase ) return;
    Rim3dView* view = RiaApplication::instance()->activeMainOrComparisonGridView();
    if ( !view ) return;

    // Create Plot
    const bool      showAfterCreation = true;
    const QString   name              = plotDescription();
    RimWellLogPlot* newPlot           = RicNewWellLogPlotFeatureImpl::createWellLogPlot( showAfterCreation, name );
    newPlot->setNamingMethod( RiaDefines::ObjectNamingMethod::CUSTOM );
    newPlot->nameConfig()->setCustomName( name );

    // Create curve tracks
    const bool       doUpdateAfter = true;
    RimWellLogTrack* wellLogExtractionDisplacementTrack =
        RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack( doUpdateAfter, QString( "Fault Reactivation Displacement Curves" ), newPlot );
    RimWellLogTrack* wellLogCalculatedTrack =
        RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack( doUpdateAfter, QString( "Fault Reactivation Displacement Difference" ), newPlot );
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

    // Create well log calculated curve for m_faceAWellPath and m_faceBWellPath
    RimWellLogCalculatedCurve* wellLogCalculatedCurve = RicWellLogTools::addWellLogCalculatedCurve( wellLogCalculatedTrack );
    wellLogCalculatedCurve->setOperator( RimWellLogCalculatedCurve::Operators::SUBTRACT );
    wellLogCalculatedCurve->setWellLogCurves( faceADisplacementCurve, faceBDisplacementCurve );
    wellLogCalculatedCurve->loadDataAndUpdate( true );
    wellLogCalculatedCurve->updateConnectedEditors();

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
    const int idx = 0;
    if ( !partCollection ) return idx;

    // Find candidates for intersected global elements
    const cvf::BoundingBox intersectingBb( point, point );
    std::vector<size_t>    intersectedGlobalElementIndexCandidates;
    partCollection->findIntersectingGlobalElementIndices( intersectingBb, &intersectedGlobalElementIndexCandidates );

    if ( intersectedGlobalElementIndexCandidates.empty() ) return idx;

    // Iterate through global element candidates and check if point is in hexCorners
    for ( const auto& globalElementIndex : intersectedGlobalElementIndexCandidates )
    {
        const auto [part, elementIndex] = partCollection->partAndElementIndex( globalElementIndex );

        // Find nodes from element
        std::array<cvf::Vec3d, 8> coordinates;
        const bool                isSuccess = part->fillElementCoordinates( elementIndex, coordinates );
        if ( !isSuccess ) continue;

        const bool isPointInCell = RigHexIntersectionTools::isPointInCell( point, coordinates.data() );
        if ( isPointInCell ) return part->elementPartId();
    }

    // Utilize first part to have an id
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
    auto geomCase = geoMechCase();
    if ( !geomCase ) return nullptr;

    Rim3dView* view = RiaApplication::instance()->activeMainOrComparisonGridView();
    if ( !view ) return nullptr;

    const int  branchIndex        = -1;
    const bool useBranchDetection = false;
    const bool updateParentPlot   = true;

    RimWellLogExtractionCurve* wellLogExtractionCurve =
        RicWellLogTools::addWellLogExtractionCurve( track, geomCase, view, wellPath, nullptr, branchIndex, useBranchDetection );
    wellLogExtractionCurve->setGeoMechResultAddress( resultAddress );
    wellLogExtractionCurve->setGeoMechPart( partId );
    wellLogExtractionCurve->updateConnectedEditors();
    wellLogExtractionCurve->loadDataAndUpdate( updateParentPlot );

    return wellLogExtractionCurve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGeoMechFaultReactivationResult::plotDescription() const
{
    RimWellLogPlotCollection* wellLogPlotCollection = RimMainPlotCollection::current()->wellLogPlotCollection();
    QString                   plotDescription       = "Fault Reactivation Plot";

    if ( !wellLogPlotCollection ) return plotDescription;

    int count = 0;
    for ( const auto& plot : wellLogPlotCollection->wellLogPlots() )
    {
        if ( plot->description().startsWith( plotDescription ) ) ++count;
    }

    return count == 0 ? plotDescription : QString( "%1 %2" ).arg( plotDescription ).arg( count );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechCase* RimGeoMechFaultReactivationResult::geoMechCase() const
{
    return firstAncestorOrThisOfTypeAsserted<RimGeoMechCase>();
}
