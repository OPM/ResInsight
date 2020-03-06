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
#include "RimOilField.h"
#include "RimProject.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimStimPlanColors.h"
#include "RimStimPlanFractureTemplate.h"
#include "RimWellPath.h"

#include "RivWellFracturePartMgr.h"

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

CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimFractureModel, "Fracture" );

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

    CAF_PDM_InitFieldNoDefault( &m_anchorPosition, "AnchorPosition", "AnchorPosition", "", "", "" );
    m_anchorPosition.uiCapability()->setUiReadOnly( true );
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
    // if ( changedField == &m_azimuth || changedField == this->objectToggleField() || changedField == &m_dip ||
    //      changedField == &m_tilt )

    if ( changedField == &m_MD )
    {
        updatePositionFromMeasuredDepth();
    }

    if ( changedField == &m_thicknessType )
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
cvf::Mat4d RimFractureModel::transformMatrix() const
{
    // cvf::Vec3d center = anchorPosition();

    // // Azimuth rotation
    // cvf::Mat4d azimuthRotation = cvf::Mat4d::fromRotation( cvf::Vec3d::Z_AXIS, cvf::Math::toRadians( -m_azimuth() -
    // 90 ) );

    // cvf::Mat4d m = azimuthRotation * rotationFromTesselator * dipRotation * tiltRotation;
    // m.setTranslation( center );

    cvf::Mat4d m;
    return m;
}

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// void RimFractureModel::triangleGeometry( std::vector<cvf::uint>* triangleIndices, std::vector<cvf::Vec3f>* nodeCoords
// ) const
// {
//     RimFractureModelTemplate* fractureDef = fractureTemplate();
//     if ( fractureDef )
//     {
//         fractureDef->fractureTriangleGeometry( nodeCoords, triangleIndices );
//     }

//     cvf::Mat4d m = transformMatrix();

//     for ( cvf::Vec3f& v : *nodeCoords )
//     {
//         cvf::Vec3d vd( v );

//         vd.transformPoint( m );

//         v = cvf::Vec3f( vd );
//     }
// }

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
    cvf::Vec3d direction( 0.0, 0.0, 1.0 );

    if ( m_thicknessType() == TRUE_STRATIGRAPHIC_THICKNESS )
    {
        direction = calculateTSTDirection();
    }

    m_dip  = cvf::Math::toDegrees( cvf::GeometryTools::getAngle( cvf::Vec3d::Z_AXIS, direction ) );
    m_tilt = cvf::Math::toDegrees( cvf::GeometryTools::getAngle( cvf::Vec3d::X_AXIS, direction ) );
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

    cvf::Vec3d boundingBoxSize( 50, 50, 100 );

    // Find upper face of cells close to the anchor point
    cvf::BoundingBox    boundingBox( m_anchorPosition() - boundingBoxSize, m_anchorPosition() + boundingBoxSize );
    std::vector<size_t> closeCells;
    mainGrid->findIntersectingCells( boundingBox, &closeCells );
    std::cout << "Close cells: " << closeCells.size() << std::endl;

    // The stratigraphic thickness is average the averge of normals of the top face
    for ( size_t globalCellIndex : closeCells )
    {
        const RigCell& cell = mainGrid->globalCellArray()[globalCellIndex];

        if ( cell.isInvalid() ) continue;

        direction += ( cell.center() - cell.faceCenter( cvf::StructGridInterface::NEG_K ) );
    }

    return direction;
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
