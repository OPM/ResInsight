/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RimSimWellFracture.h"

#include "RigCell.h"
#include "RigMainGrid.h"
#include "RigSimWellData.h"

#include "RimEclipseView.h"
#include "RimEllipseFractureTemplate.h"
#include "RimFracture.h"
#include "RimFractureTemplate.h"
#include "RimProject.h"
#include "RimSimWellInView.h"

#include "RigWellPath.h"
#include "cafPdmUiDoubleSliderEditor.h"

CAF_PDM_SOURCE_INIT( RimSimWellFracture, "SimWellFracture" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSimWellFracture::RimSimWellFracture( void )
{
    CAF_PDM_InitObject( "SimWellFracture", ":/FractureSymbol16x16.png", "", "" );

    CAF_PDM_InitField( &m_location, "MeasuredDepth", 0.0f, "Pseudo Length Location", "", "", "" );
    m_location.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_displayIJK, "Cell_IJK", "Cell IJK", "", "", "" );
    m_displayIJK.registerGetMethod( this, &RimSimWellFracture::createOneBasedIJKText );
    m_displayIJK.uiCapability()->setUiReadOnly( true );
    m_displayIJK.xmlCapability()->disableIO();

    CAF_PDM_InitField( &m_branchIndex, "Branch", 0, "Branch", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSimWellFracture::~RimSimWellFracture()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSimWellFracture::setClosestWellCoord( cvf::Vec3d& position, size_t branchIndex )
{
    computeSimWellBranchesIfRequired();

    double location = m_branchCenterLines[branchIndex].locationAlongWellCoords( position );

    m_branchIndex = static_cast<int>( branchIndex );

    m_location = location;
    updateFracturePositionFromLocation();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSimWellFracture::updateAzimuthBasedOnWellAzimuthAngle()
{
    computeSimWellBranchesIfRequired();

    if ( !fractureTemplate() ) return;

    if ( fractureTemplate()->orientationType() == RimFractureTemplate::ALONG_WELL_PATH ||
         fractureTemplate()->orientationType() == RimFractureTemplate::TRANSVERSE_WELL_PATH )
    {
        double simWellAzimuth = wellAzimuthAtFracturePosition();

        if ( fractureTemplate()->orientationType() == RimFractureTemplate::ALONG_WELL_PATH )
        {
            m_azimuth = simWellAzimuth;
        }
        else if ( fractureTemplate()->orientationType() == RimFractureTemplate::TRANSVERSE_WELL_PATH )
        {
            if ( simWellAzimuth + 90 < 360 )
                m_azimuth = simWellAzimuth + 90;
            else
                m_azimuth = simWellAzimuth - 90;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimSimWellFracture::wellAzimuthAtFracturePosition() const
{
    double simWellAzimuth = m_branchCenterLines[m_branchIndex].simWellAzimuthAngle( fracturePosition() );
    if ( simWellAzimuth < 0 ) simWellAzimuth += 360;

    return simWellAzimuth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSimWellFracture::loadDataAndUpdate()
{
    computeSimWellBranchCenterLines();
    updateFracturePositionFromLocation();
    updateAzimuthBasedOnWellAzimuthAngle();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RimSimWellFracture::perforationLengthCenterLineCoords() const
{
    std::vector<cvf::Vec3d> coords;

    if ( !m_branchCenterLines.empty() && m_branchIndex < static_cast<int>( m_branchCenterLines.size() ) )
    {
        RigWellPath wellPathGeometry;

        wellPathGeometry.m_wellPathPoints = m_branchCenterLines[m_branchIndex].wellPathPoints();
        wellPathGeometry.m_measuredDepths = m_branchCenterLines[m_branchIndex].measuredDepths();

        double startMd = m_location - perforationLength() / 2.0;
        double endMd   = m_location + perforationLength() / 2.0;

        coords = wellPathGeometry.clippedPointSubset( startMd, endMd ).first;
    }

    return coords;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSimWellFracture::isEnabled() const
{
    return isChecked();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSimWellFracture::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                           const QVariant&            oldValue,
                                           const QVariant&            newValue )
{
    RimFracture::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_location || changedField == &m_branchIndex )
    {
        updateFracturePositionFromLocation();

        RimFractureTemplate::FracOrientationEnum orientation;
        if ( fractureTemplate() )
            orientation = fractureTemplate()->orientationType();
        else
            orientation = RimFractureTemplate::AZIMUTH;

        if ( orientation != RimFractureTemplate::AZIMUTH )
        {
            updateAzimuthBasedOnWellAzimuthAngle();
        }

        RimProject* proj;
        this->firstAncestorOrThisOfType( proj );
        if ( proj ) proj->reloadCompletionTypeResultsInAllViews();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSimWellFracture::recomputeWellCenterlineCoordinates()
{
    m_branchCenterLines.clear();

    computeSimWellBranchesIfRequired();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSimWellFracture::updateFracturePositionFromLocation()
{
    computeSimWellBranchesIfRequired();

    if ( m_branchCenterLines.size() > 0 )
    {
        cvf::Vec3d interpolated = m_branchCenterLines[m_branchIndex()].interpolatedPointAlongWellPath( m_location() );

        this->setAnchorPosition( interpolated );

        RimProject* proj;
        this->firstAncestorOrThisOfType( proj );
        if ( proj ) proj->scheduleCreateDisplayModelAndRedrawAllViews();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSimWellFracture::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimFracture::defineUiOrdering( uiConfigName, uiOrdering );

    uiOrdering.add( nameField() );
    uiOrdering.add( &m_fractureTemplate );

    caf::PdmUiGroup* locationGroup = uiOrdering.addNewGroup( "Location / Orientation" );
    locationGroup->add( &m_location );
    locationGroup->add( &m_branchIndex );
    locationGroup->add( &m_azimuth );
    locationGroup->add( &m_uiWellPathAzimuth );
    locationGroup->add( &m_uiWellFractureAzimuthDiff );
    locationGroup->add( &m_wellFractureAzimuthAngleWarning );
    locationGroup->add( &m_dip );
    locationGroup->add( &m_tilt );

    caf::PdmUiGroup* propertyGroup = uiOrdering.addNewGroup( "Properties" );
    propertyGroup->add( &m_fractureUnit );
    propertyGroup->add( &m_stimPlanTimeIndexToPlot );
    propertyGroup->add( &m_perforationLength );
    propertyGroup->add( &m_perforationEfficiency );
    propertyGroup->add( &m_wellDiameter );

    caf::PdmUiGroup* fractureCenterGroup = uiOrdering.addNewGroup( "Fracture Center Info" );
    fractureCenterGroup->add( &m_uiAnchorPosition );
    fractureCenterGroup->add( &m_displayIJK );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSimWellFracture::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                QString                    uiConfigName,
                                                caf::PdmUiEditorAttribute* attribute )
{
    RimFracture::defineEditorAttribute( field, uiConfigName, attribute );

    if ( field == &m_location )
    {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute );

        if ( myAttr )
        {
            computeSimWellBranchesIfRequired();

            if ( m_branchCenterLines.size() > 0 )
            {
                const RigSimulationWellCoordsAndMD& pointAndMd = m_branchCenterLines[m_branchIndex];

                myAttr->m_minimum         = pointAndMd.measuredDepths().front();
                myAttr->m_maximum         = pointAndMd.measuredDepths().back();
                myAttr->m_sliderTickCount = pointAndMd.measuredDepths().back();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSimWellFracture::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                         bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options = RimFracture::calculateValueOptions( fieldNeedingOptions, useOptionsOnly );

    if ( fieldNeedingOptions == &m_branchIndex )
    {
        if ( m_branchCenterLines.size() == 0 )
        {
            computeSimWellBranchesIfRequired();
        }

        if ( m_branchCenterLines.size() > 0 )
        {
            size_t branchCount = m_branchCenterLines.size();

            for ( size_t bIdx = 0; bIdx < branchCount; ++bIdx )
            {
                // Use 1-based index in UI
                options.push_back( caf::PdmOptionItemInfo( QString::number( bIdx + 1 ), QVariant::fromValue( bIdx ) ) );
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigMainGrid* RimSimWellFracture::ownerCaseMainGrid() const
{
    RimEclipseView* ownerEclView;
    this->firstAncestorOrThisOfType( ownerEclView );

    if ( ownerEclView )
        return ownerEclView->mainGrid();
    else
        return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSimWellFracture::computeSimWellBranchesIfRequired()
{
    if ( m_branchCenterLines.size() == 0 )
    {
        computeSimWellBranchCenterLines();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSimWellFracture::computeSimWellBranchCenterLines()
{
    m_branchCenterLines.clear();

    RimSimWellInView* rimWell = nullptr;
    this->firstAncestorOrThisOfType( rimWell );
    CVF_ASSERT( rimWell );

    std::vector<std::vector<cvf::Vec3d>>         pipeBranchesCLCoords;
    std::vector<std::vector<RigWellResultPoint>> pipeBranchesCellIds;

    rimWell->calculateWellPipeStaticCenterLine( pipeBranchesCLCoords, pipeBranchesCellIds );

    for ( const auto& branch : pipeBranchesCLCoords )
    {
        RigSimulationWellCoordsAndMD wellPathWithMD( branch );

        m_branchCenterLines.push_back( wellPathWithMD );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSimWellFracture::createOneBasedIJKText() const
{
    RigMainGrid* mainGrid = ownerCaseMainGrid();
    size_t       i, j, k;
    size_t       anchorCellIdx = mainGrid->findReservoirCellIndexFromPoint( anchorPosition() );

    if ( anchorCellIdx == cvf::UNDEFINED_SIZE_T ) return "";

    size_t             gridLocalCellIdx;
    const RigGridBase* hostGrid = mainGrid->gridAndGridLocalIdxFromGlobalCellIdx( anchorCellIdx, &gridLocalCellIdx );

    bool ok = hostGrid->ijkFromCellIndex( gridLocalCellIdx, &i, &j, &k );
    if ( !ok ) return "";

    return QString( "Grid %1: [%2, %3, %4]" )
        .arg( QString::fromStdString( hostGrid->gridName() ) )
        .arg( i + 1 )
        .arg( j + 1 )
        .arg( k + 1 );
}
