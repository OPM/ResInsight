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

    CAF_PDM_InitField( &m_orientationType,
                       "Orientation",
                       caf::AppEnum<RimFractureTemplate::FracOrientationEnum>( RimFractureTemplate::TRANSVERSE_WELL_PATH ),
                       "Fracture Orientation",
                       "",
                       "",
                       "" );

    CAF_PDM_InitField( &m_azimuth, "Azimuth", 0.0, "Azimuth", "", "", "" );
    m_azimuth.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_dip, "Dip", 0.0, "Dip", "", "", "" );
    m_dip.uiCapability()->setUiReadOnly( true );
    CAF_PDM_InitField( &m_tilt, "Tilt", 0.0, "Tilt", "", "", "" );
    m_tilt.uiCapability()->setUiReadOnly( true );

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
cvf::Mat4d RimFractureModel::transformMatrix() const
{
    // Ellipsis geometry is produced in XY-plane, rotate 90 deg around X to get zero azimuth along Y
    cvf::Mat4d rotationFromTesselator = cvf::Mat4d::fromRotation( cvf::Vec3d::Y_AXIS, cvf::Math::toRadians( 90.0f ) );

    cvf::Mat4d directionRotation = rotationMatrixBetweenVectors( cvf::Vec3d::Z_AXIS, m_thicknessDirection() );

    cvf::Mat4d m = directionRotation * rotationFromTesselator;
    m.setTranslation( anchorPosition() );

    return m;
}

//--------------------------------------------------------------------------------------------------
/// Taken from OverlayNavigationCube::computeNewUpVector
/// Consider move to geometry util class
//--------------------------------------------------------------------------------------------------
cvf::Mat4d RimFractureModel::rotationMatrixBetweenVectors( const cvf::Vec3d& v1, const cvf::Vec3d& v2 )
{
    cvf::Vec3d rotAxis = v1 ^ v2;
    rotAxis.normalize();

    // Guard acos against out-of-domain input
    const double dotProduct = cvf::Math::clamp( v1 * v2, -1.0, 1.0 );
    const double angle      = cvf::Math::acos( dotProduct );
    return cvf::Mat4d::fromRotation( rotAxis, angle );
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

    m_dip     = cvf::Math::toDegrees( cvf::GeometryTools::getAngle( cvf::Vec3d::Z_AXIS, direction ) );
    m_tilt    = cvf::Math::toDegrees( cvf::GeometryTools::getAngle( cvf::Vec3d::X_AXIS, direction ) );
    m_azimuth = cvf::Math::toDegrees( cvf::GeometryTools::getAngle( cvf::Vec3d::Y_AXIS, direction ) );
}

cvf::Vec3d RimFractureModel::calculateTSTDirection() const
{
    cvf::Vec3d direction = cvf::Vec3d::ZERO;

    // TODO: find a better way?
    // Find an eclipse case
    RiaApplication* app  = RiaApplication::instance();
    RimProject*     proj = app->project();
    if ( proj->eclipseCases().empty() ) return direction;

    RimEclipseCase* eclipseCase = proj->eclipseCases()[0];
    if ( !eclipseCase ) return direction;

    RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();
    if ( !eclipseCaseData ) return direction;

    RigMainGrid* mainGrid = eclipseCaseData->mainGrid();
    if ( !mainGrid ) return direction;

    cvf::Vec3d boundingBoxSize( m_boundingBoxHorizontal, m_boundingBoxHorizontal, m_boundingBoxVertical );

    // Find upper face of cells close to the anchor point
    cvf::BoundingBox    boundingBox( m_anchorPosition() - boundingBoxSize, m_anchorPosition() + boundingBoxSize );
    std::vector<size_t> closeCells;
    mainGrid->findIntersectingCells( boundingBox, &closeCells );
    std::cout << "Close cells count: " << closeCells.size() << std::endl;

    if ( closeCells.empty() )
    {
        // No close cells found: just point straight up
        return cvf::Vec3d( 0.0, 0.0, -1.0 );
    }

    // The stratigraphic thickness is average the averge of normals of the top face
    for ( size_t globalCellIndex : closeCells )
    {
        const RigCell& cell = mainGrid->globalCellArray()[globalCellIndex];

        if ( cell.isInvalid() ) continue;

        direction += ( cell.center() - cell.faceCenter( cvf::StructGridInterface::NEG_K ) ).getNormalized();
    }

    return ( direction / static_cast<double>( closeCells.size() ) ).getNormalized();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModel::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    m_azimuth.uiCapability()->setUiHidden( m_orientationType != RimFractureTemplate::AZIMUTH );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModel::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                              QString                    uiConfigName,
                                              caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_azimuth )
    {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->m_minimum = 0;
            myAttr->m_maximum = 360;
        }
    }
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

// TODO: replace with logging!!!!
#include <sstream>
std::string toString2( const cvf::Vec3d& vec )
{
    std::stringstream stream;
    stream << "[" << vec.x() << " " << vec.y() << " " << vec.z() << "]";
    return stream.str();
}

//==================================================================================================
///
//==================================================================================================
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

    std::cout << "All cells bounding box: " << toString2( geometryBoundingBox.min() ) << " "
              << toString2( geometryBoundingBox.max() ) << std::endl;
    std::cout << "Position: " << toString2( position ) << std::endl;
    std::cout << "Direction: " << toString2( direction ) << std::endl;

    // Create plane on top and bottom of formation
    cvf::Plane topPlane;
    topPlane.setFromPointAndNormal( geometryBoundingBox.max(), cvf::Vec3d::Z_AXIS );
    cvf::Plane bottomPlane;
    bottomPlane.setFromPointAndNormal( geometryBoundingBox.min(), cvf::Vec3d::Z_AXIS );

    // Convert direction up for z
    cvf::Vec3d directionUp( direction.x(), direction.y(), -direction.z() );

    // Find and add point on top plane
    cvf::Vec3d abovePlane = position + ( directionUp * 10000.0 );
    topPlane.intersect( position, abovePlane, &topPosition );
    double topMD = 0.0;
    // m_wellPath->m_wellPathPoints.push_back( topPosition );
    // m_wellPath->m_measuredDepths.push_back( topMD );
    std::cout << "TOP:    " << toString2( topPosition ) << " MD: " << topMD << std::endl;

    // The anchor position
    double dist = ( topPosition - position ).length();
    // m_wellPath->m_wellPathPoints.push_back( position );
    // m_wellPath->m_measuredDepths.push_back( dist );
    std::cout << "ANCHOR: " << toString2( position ) << " MD: " << dist << std::endl;

    // Find and add point on bottom plane
    cvf::Vec3d belowPlane = position + ( directionUp * -10000.0 );
    bottomPlane.intersect( position, belowPlane, &bottomPosition );

    double dist2 = ( topPosition - bottomPosition ).length();
    // m_wellPath->m_wellPathPoints.push_back( bottomPosition );
    // m_wellPath->m_measuredDepths.push_back( dist2 );
    std::cout << "BOTTOM: " << toString2( bottomPosition ) << " MD: " << dist2 << std::endl;
}
