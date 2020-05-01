/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RimFractureModel.h"

#include "RiaApplication.h"
#include "RiaColorTables.h"
#include "RiaCompletionTypeCalculationScheduler.h"
#include "RiaEclipseUnitTools.h"
#include "RiaLogging.h"

#include "Riu3DMainWindowTools.h"

#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigWellPath.h"

#include "Rim3dView.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimEllipseFractureTemplate.h"
#include "RimModeledWellPath.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimStimPlanColors.h"
#include "RimStimPlanFractureTemplate.h"
#include "RimWellPath.h"
#include "RimWellPathGeometryDef.h"
#include "RimWellPathTarget.h"

#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiToolButtonEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include "cvfBoundingBox.h"
#include "cvfGeometryTools.h"
#include "cvfMath.h"
#include "cvfMatrix4.h"
#include "cvfPlane.h"

#include <cmath>

CAF_PDM_SOURCE_INIT( RimFractureModel, "RimFractureModel" );

namespace caf
{
template <>
void caf::AppEnum<RimFractureModel::ThicknessType>::setUp()
{
    addItem( RimFractureModel::TRUE_VERTICAL_THICKNESS, "TVT", "True Vertical Thickness" );
    addItem( RimFractureModel::TRUE_STRATIGRAPHIC_THICKNESS, "TST", "True Stratigraphic Thickness" );

    setDefault( RimFractureModel::TRUE_VERTICAL_THICKNESS );
}
}; // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModel::RimFractureModel()
{
    CAF_PDM_InitObject( "FractureModel", "", "", "" );

    CAF_PDM_InitField( &m_MD, "MD", 0.0, "MD", "", "", "" );

    CAF_PDM_InitField( &m_thicknessType,
                       "ThicknessType",
                       caf::AppEnum<ThicknessType>( TRUE_STRATIGRAPHIC_THICKNESS ),
                       "Thickness Type",
                       "",
                       "",
                       "" );

    CAF_PDM_InitFieldNoDefault( &m_anchorPosition, "AnchorPosition", "Anchor Position", "", "", "" );
    m_anchorPosition.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_thicknessDirection, "ThicknessDirection", "Thickness Direction", "", "", "" );
    m_thicknessDirection.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_thicknessDirectionWellPath,
                                "ThicknessDirectionWellPath",
                                "Thickness Direction Well Path",
                                "",
                                "",
                                "" );

    CAF_PDM_InitField( &m_boundingBoxHorizontal, "BoundingBoxHorizontal", 50.0, "Bounding Box Horizontal", "", "", "" );
    CAF_PDM_InitField( &m_boundingBoxVertical, "BoundingBoxVertical", 100.0, "Bounding Box Vertical", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModel::~RimFractureModel()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFractureModel::isEnabled() const
{
    return isChecked();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModel::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                         const QVariant&            oldValue,
                                         const QVariant&            newValue )
{
    if ( changedField == &m_MD )
    {
        updatePositionFromMeasuredDepth();
    }

    if ( changedField == &m_MD || changedField == &m_thicknessType || changedField == &m_boundingBoxVertical ||
         changedField == &m_boundingBoxHorizontal )
    {
        updateThicknessDirection();
    }

    {
        RimEclipseCase* eclipseCase = nullptr;
        this->firstAncestorOrThisOfType( eclipseCase );
        if ( eclipseCase )
        {
            RiaCompletionTypeCalculationScheduler::instance()->scheduleRecalculateCompletionTypeAndRedrawAllViews(
                eclipseCase );
        }
        else
        {
            RiaCompletionTypeCalculationScheduler::instance()->scheduleRecalculateCompletionTypeAndRedrawAllViews();
        }

        RiaApplication::instance()->project()->scheduleCreateDisplayModelAndRedrawAllViews();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimFractureModel::fracturePosition() const
{
    return m_anchorPosition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::WellPathComponentType RimFractureModel::componentType() const
{
    return RiaDefines::WellPathComponentType::FRACTURE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFractureModel::componentLabel() const
{
    return name();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFractureModel::componentTypeLabel() const
{
    return "Fracture Model";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimFractureModel::defaultComponentColor() const
{
    return cvf::Color3f::RED; // RiaColorTables::wellPathComponentColors()[componentType()];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::startMD() const
{
    return m_MD();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::endMD() const
{
    return m_MD();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimFractureModel::boundingBoxInDomainCoords() const
{
    // std::vector<cvf::Vec3f> nodeCoordVec;
    // std::vector<cvf::uint>  triangleIndices;

    // this->triangleGeometry( &triangleIndices, &nodeCoordVec );

    cvf::BoundingBox fractureBBox;
    // for ( const auto& nodeCoord : nodeCoordVec )
    //     fractureBBox.add( nodeCoord );

    return fractureBBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimFractureModel::anchorPosition() const
{
    return m_anchorPosition();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimFractureModel::thicknessDirection() const
{
    return m_thicknessDirection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModel::updatePositionFromMeasuredDepth()
{
    cvf::Vec3d positionAlongWellpath = cvf::Vec3d::ZERO;

    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>( this );
    if ( !objHandle ) return;

    RimWellPath* wellPath = nullptr;
    objHandle->firstAncestorOrThisOfType( wellPath );
    if ( !wellPath ) return;

    RigWellPath* wellPathGeometry = wellPath->wellPathGeometry();
    if ( wellPathGeometry )
    {
        positionAlongWellpath = wellPathGeometry->interpolatedPointAlongWellPath( m_MD() );
    }

    m_anchorPosition = positionAlongWellpath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModel::updateThicknessDirection()
{
    // True vertical thickness: just point straight up
    cvf::Vec3d direction( 0.0, 0.0, -1.0 );

    if ( m_thicknessType() == TRUE_STRATIGRAPHIC_THICKNESS )
    {
        direction = calculateTSTDirection();
    }

    m_thicknessDirection = direction;

    if ( m_thicknessDirectionWellPath )
    {
        cvf::Vec3d topPosition;
        cvf::Vec3d bottomPosition;

        findThicknessTargetPoints( topPosition, bottomPosition );
        topPosition.z() *= -1.0;
        bottomPosition.z() *= -1.0;

        RimWellPathGeometryDef* wellGeomDef = m_thicknessDirectionWellPath->geometryDefinition();
        wellGeomDef->deleteAllTargets();

        RimWellPathTarget* topPathTarget = new RimWellPathTarget();

        topPathTarget->setAsPointTargetXYD( topPosition );
        RimWellPathTarget* bottomPathTarget = new RimWellPathTarget();
        bottomPathTarget->setAsPointTargetXYD( bottomPosition );

        wellGeomDef->insertTarget( nullptr, topPathTarget );
        wellGeomDef->insertTarget( nullptr, bottomPathTarget );

        wellGeomDef->updateConnectedEditors();
        wellGeomDef->updateWellPathVisualization();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimFractureModel::calculateTSTDirection() const
{
    cvf::Vec3d defaultDirection = cvf::Vec3d( 0.0, 0.0, -1.0 );

    // TODO: find a better way?
    // Find an eclipse case
    RiaApplication* app  = RiaApplication::instance();
    RimProject*     proj = app->project();
    if ( proj->eclipseCases().empty() ) return defaultDirection;

    RimEclipseCase* eclipseCase = proj->eclipseCases()[0];
    if ( !eclipseCase ) return defaultDirection;

    RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();
    if ( !eclipseCaseData ) return defaultDirection;

    RigMainGrid* mainGrid = eclipseCaseData->mainGrid();
    if ( !mainGrid ) return defaultDirection;

    cvf::Vec3d boundingBoxSize( m_boundingBoxHorizontal, m_boundingBoxHorizontal, m_boundingBoxVertical );

    // Find upper face of cells close to the anchor point
    cvf::BoundingBox    boundingBox( m_anchorPosition() - boundingBoxSize, m_anchorPosition() + boundingBoxSize );
    std::vector<size_t> closeCells;
    mainGrid->findIntersectingCells( boundingBox, &closeCells );

    // The stratigraphic thickness is the averge of normals of the top face
    cvf::Vec3d direction = cvf::Vec3d::ZERO;

    int numContributingCells = 0;
    for ( size_t globalCellIndex : closeCells )
    {
        const RigCell& cell = mainGrid->globalCellArray()[globalCellIndex];

        if ( !cell.isInvalid() )
        {
            direction += cell.faceNormalWithAreaLenght( cvf::StructGridInterface::NEG_K ).getNormalized();
            numContributingCells++;
        }
    }

    RiaLogging::info( QString( "TST contributing cells: %1/%2" ).arg( numContributingCells ).arg( closeCells.size() ) );
    if ( numContributingCells == 0 )
    {
        // No valid close cells found: just point straight up
        return defaultDirection;
    }

    return ( direction / static_cast<double>( numContributingCells ) ).getNormalized();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModel::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    m_thicknessDirectionWellPath.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RimFractureModel::wellPath() const
{
    const caf::PdmObjectHandle* objHandle = dynamic_cast<const caf::PdmObjectHandle*>( this );
    if ( !objHandle ) return nullptr;

    RimWellPath* wellPath = nullptr;
    objHandle->firstAncestorOrThisOfType( wellPath );
    return wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimModeledWellPath* RimFractureModel::thicknessDirectionWellPath() const
{
    return m_thicknessDirectionWellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModel::setThicknessDirectionWellPath( RimModeledWellPath* thicknessDirectionWellPath )
{
    m_thicknessDirectionWellPath = thicknessDirectionWellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFractureModel::vecToString( const cvf::Vec3d& vec )
{
    return QString( "[%1, %2, %3]" ).arg( vec.x() ).arg( vec.y() ).arg( vec.z() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModel::findThicknessTargetPoints( cvf::Vec3d& topPosition, cvf::Vec3d& bottomPosition )
{
    // TODO: duplicated and ugly!
    RiaApplication* app  = RiaApplication::instance();
    RimProject*     proj = app->project();
    if ( proj->eclipseCases().empty() ) return;

    RimEclipseCase* eclipseCase = proj->eclipseCases()[0];
    if ( !eclipseCase ) return;

    RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();
    if ( !eclipseCaseData ) return;

    const cvf::Vec3d& position  = anchorPosition();
    const cvf::Vec3d& direction = thicknessDirection();

    // Create a "fake" well path which from top to bottom of formation
    // passing through the point and with the given direction

    const cvf::BoundingBox& geometryBoundingBox = eclipseCase->mainGrid()->boundingBox();

    RiaLogging::info( QString( "All cells bounding box: %1 %2" )
                          .arg( RimFractureModel::vecToString( geometryBoundingBox.min() ) )
                          .arg( RimFractureModel::vecToString( geometryBoundingBox.max() ) ) );
    RiaLogging::info( QString( "Position:  %1" ).arg( RimFractureModel::vecToString( position ) ) );
    RiaLogging::info( QString( "Direction: %1" ).arg( RimFractureModel::vecToString( direction ) ) );

    // Create plane on top and bottom of formation
    cvf::Plane topPlane;
    topPlane.setFromPointAndNormal( geometryBoundingBox.max(), cvf::Vec3d::Z_AXIS );
    cvf::Plane bottomPlane;
    bottomPlane.setFromPointAndNormal( geometryBoundingBox.min(), cvf::Vec3d::Z_AXIS );

    // Find and add point on top plane
    cvf::Vec3d abovePlane = position + ( direction * -10000.0 );
    topPlane.intersect( position, abovePlane, &topPosition );
    RiaLogging::info( QString( "Top: %1" ).arg( RimFractureModel::vecToString( topPosition ) ) );

    // Find and add point on bottom plane
    cvf::Vec3d belowPlane = position + ( direction * 10000.0 );
    bottomPlane.intersect( position, belowPlane, &bottomPosition );
    RiaLogging::info( QString( "Bottom: %1" ).arg( RimFractureModel::vecToString( bottomPosition ) ) );
}
