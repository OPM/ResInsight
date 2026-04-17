/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026   Equinor ASA
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

#include "RimRefinementRegion.h"

#include "ExportCommands/RicRefinementSettings.h"

#include "RigMainGrid.h"

#include "Rim3dView.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"

#include "cafPdmFieldCvfColor.h"
#include "cafPdmUiSliderEditor.h"

CAF_PDM_SOURCE_INIT( RimRefinementRegion, "RefinementRegion" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRefinementRegion::RimRefinementRegion()
{
    CAF_PDM_InitObject( "Refinement Region", ":/CellFilter_Range.png" );

    CAF_PDM_InitField( &m_isActive, "IsActive", true, "Show in 3D View" );
    CAF_PDM_InitField( &m_regionName, "Name", QString( "Refinement Region" ), "Name" );

    CAF_PDM_InitField( &m_startI, "StartI", 1, "I Start" );
    m_startI.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_cellCountI, "CellCountI", 1, "I Width" );
    m_cellCountI.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_startJ, "StartJ", 1, "J Start" );
    m_startJ.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_cellCountJ, "CellCountJ", 1, "J Width" );
    m_cellCountJ.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_startK, "StartK", 1, "K Start" );
    m_startK.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_cellCountK, "CellCountK", 1, "K Width" );
    m_cellCountK.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_previewColor, "PreviewColor", cvf::Color3f( 1.0f, 0.5f, 0.0f ), "Preview Color" );

    CAF_PDM_InitFieldNoDefault( &m_refinementSettings, "RefinementSettings", "Refinement" );
    m_refinementSettings = new RicRefinementSettings();
    m_refinementSettings->uiCapability()->setUiTreeHidden( true );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimRefinementRegion::isActive() const
{
    return m_isActive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimRefinementRegion::regionName() const
{
    return m_regionName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRefinementRegion::setRegionName( const QString& name )
{
    m_regionName = name;
    setUiName( name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimRefinementRegion::startI() const
{
    return m_startI();
}

int RimRefinementRegion::startJ() const
{
    return m_startJ();
}

int RimRefinementRegion::startK() const
{
    return m_startK();
}

int RimRefinementRegion::cellCountI() const
{
    return m_cellCountI();
}

int RimRefinementRegion::cellCountJ() const
{
    return m_cellCountJ();
}

int RimRefinementRegion::cellCountK() const
{
    return m_cellCountK();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::VecIjk0 RimRefinementRegion::ijkMin() const
{
    return caf::VecIjk0( static_cast<size_t>( std::max( 1, m_startI() ) - 1 ),
                         static_cast<size_t>( std::max( 1, m_startJ() ) - 1 ),
                         static_cast<size_t>( std::max( 1, m_startK() ) - 1 ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::VecIjk0 RimRefinementRegion::ijkMax() const
{
    int endI = m_startI() + std::max( 1, m_cellCountI() ) - 1;
    int endJ = m_startJ() + std::max( 1, m_cellCountJ() ) - 1;
    int endK = m_startK() + std::max( 1, m_cellCountK() ) - 1;

    return caf::VecIjk0( static_cast<size_t>( std::max( 1, endI ) - 1 ),
                         static_cast<size_t>( std::max( 1, endJ ) - 1 ),
                         static_cast<size_t>( std::max( 1, endK ) - 1 ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRefinementRegion::setDefaultsFromCase( RimEclipseCase* eclipseCase )
{
    if ( !eclipseCase ) return;
    auto grid = eclipseCase->mainGrid();
    if ( !grid ) return;

    m_startI     = 1;
    m_startJ     = 1;
    m_startK     = 1;
    m_cellCountI = static_cast<int>( grid->cellCountI() );
    m_cellCountJ = static_cast<int>( grid->cellCountJ() );
    m_cellCountK = static_cast<int>( grid->cellCountK() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<RigRefinement> RimRefinementRegion::effectiveRefinement() const
{
    m_refinementSettings->setSectorBounds( ijkMin(), ijkMax() );
    return m_refinementSettings->effectiveRefinement();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicRefinementSettings* RimRefinementRegion::refinementSettings() const
{
    return m_refinementSettings();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimRefinementRegion::validateWithinSector( int sectorMinI,
                                                   int sectorMinJ,
                                                   int sectorMinK,
                                                   int sectorMaxI,
                                                   int sectorMaxJ,
                                                   int sectorMaxK ) const
{
    int endI = m_startI() + m_cellCountI() - 1;
    int endJ = m_startJ() + m_cellCountJ() - 1;
    int endK = m_startK() + m_cellCountK() - 1;

    if ( m_startI() < sectorMinI || endI > sectorMaxI || m_startJ() < sectorMinJ || endJ > sectorMaxJ || m_startK() < sectorMinK ||
         endK > sectorMaxK )
    {
        return QString( "Region '%1' (I=%2-%3, J=%4-%5, K=%6-%7) is outside the sector bounds "
                        "(I=%8-%9, J=%10-%11, K=%12-%13)." )
            .arg( m_regionName() )
            .arg( m_startI() )
            .arg( endI )
            .arg( m_startJ() )
            .arg( endJ )
            .arg( m_startK() )
            .arg( endK )
            .arg( sectorMinI )
            .arg( sectorMaxI )
            .arg( sectorMinJ )
            .arg( sectorMaxJ )
            .arg( sectorMinK )
            .arg( sectorMaxK );
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimRefinementRegion::previewColor() const
{
    return m_previewColor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimRefinementRegion::objectToggleField()
{
    return &m_isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimRefinementRegion::userDescriptionField()
{
    return &m_regionName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRefinementRegion::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto* rangeGroup = uiOrdering.addNewGroup( "Region Bounds" );
    rangeGroup->add( &m_startI );
    rangeGroup->add( &m_cellCountI );
    rangeGroup->add( &m_startJ );
    rangeGroup->add( &m_cellCountJ );
    rangeGroup->add( &m_startK );
    rangeGroup->add( &m_cellCountK );

    auto* appearanceGroup = uiOrdering.addNewGroup( "Appearance" );
    appearanceGroup->add( &m_previewColor );
    appearanceGroup->setCollapsedByDefault();

    m_refinementSettings->addToUiOrdering( uiOrdering );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRefinementRegion::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    auto* sliderAttr = dynamic_cast<caf::PdmUiSliderEditorAttribute*>( attribute );
    if ( !sliderAttr ) return;

    auto grid = mainGrid();
    if ( !grid ) return;

    if ( field == &m_startI || field == &m_cellCountI )
    {
        sliderAttr->m_minimum = 1;
        sliderAttr->m_maximum = static_cast<int>( grid->cellCountI() );
    }
    else if ( field == &m_startJ || field == &m_cellCountJ )
    {
        sliderAttr->m_minimum = 1;
        sliderAttr->m_maximum = static_cast<int>( grid->cellCountJ() );
    }
    else if ( field == &m_startK || field == &m_cellCountK )
    {
        sliderAttr->m_minimum = 1;
        sliderAttr->m_maximum = static_cast<int>( grid->cellCountK() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRefinementRegion::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_regionName )
    {
        setUiName( m_regionName() );
    }

    if ( auto grid = mainGrid() )
    {
        int maxI     = static_cast<int>( grid->cellCountI() );
        int maxJ     = static_cast<int>( grid->cellCountJ() );
        int maxK     = static_cast<int>( grid->cellCountK() );
        m_startI     = std::clamp( m_startI.v(), 1, maxI );
        m_startJ     = std::clamp( m_startJ.v(), 1, maxJ );
        m_startK     = std::clamp( m_startK.v(), 1, maxK );
        m_cellCountI = std::clamp( m_cellCountI.v(), 1, maxI - m_startI() + 1 );
        m_cellCountJ = std::clamp( m_cellCountJ.v(), 1, maxJ - m_startJ() + 1 );
        m_cellCountK = std::clamp( m_cellCountK.v(), 1, maxK - m_startK() + 1 );
    }

    if ( auto view = firstAncestorOrThisOfType<Rim3dView>() )
    {
        view->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigMainGrid* RimRefinementRegion::mainGrid() const
{
    auto view = firstAncestorOrThisOfType<RimEclipseView>();
    if ( !view ) return nullptr;
    auto eclipseCase = view->eclipseCase();
    return eclipseCase ? eclipseCase->mainGrid() : nullptr;
}
