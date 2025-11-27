/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RimCellRangeFilter.h"

#include "RiaApplication.h"
#include "RigActiveCellInfo.h"
#include "RigReservoirGridTools.h"

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimTools.h"

#include "cafPdmUiLabelEditor.h"
#include "cafPdmUiSliderEditor.h"

#include "cvfAssert.h"
#include "cvfStructGrid.h"

CAF_PDM_SOURCE_INIT( RimCellRangeFilter, "CellRangeFilter" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellRangeFilter::RimCellRangeFilter()
    : RimCellFilter( RimCellFilter::RANGE )
{
    CAF_PDM_InitObject( "Cell Range Filter", ":/CellFilter_Range.png" );

    CAF_PDM_InitField( &m_labelI, "LabelI", QString( "I" ), "I" );
    m_labelI.uiCapability()->setUiEditorTypeName( caf::PdmUiLabelEditor::uiEditorTypeName() );
    m_labelI.xmlCapability()->disableIO();

    CAF_PDM_InitField( &startIndexI, "StartIndexI", 1, "I Start" );
    startIndexI.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &cellCountI, "CellCountI", 1, "  Width" );
    cellCountI.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_labelJ, "LabelJ", QString( "J" ), "J" );
    m_labelJ.uiCapability()->setUiEditorTypeName( caf::PdmUiLabelEditor::uiEditorTypeName() );
    m_labelJ.xmlCapability()->disableIO();

    CAF_PDM_InitField( &startIndexJ, "StartIndexJ", 1, "J Start" );
    startIndexJ.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &cellCountJ, "CellCountJ", 1, "  Width" );
    cellCountJ.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_labelK, "LabelK", QString( "K" ), "K" );
    m_labelK.uiCapability()->setUiEditorTypeName( caf::PdmUiLabelEditor::uiEditorTypeName() );
    m_labelK.xmlCapability()->disableIO();

    CAF_PDM_InitField( &startIndexK, "StartIndexK", 1, "K Start" );
    startIndexK.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &cellCountK, "CellCountK", 1, "  Width" );
    cellCountK.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );

    m_propagateToSubGrids = true;

    updateIconState();
    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellRangeFilter::~RimCellRangeFilter()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCellRangeFilter::fullName() const
{
    bool isRadialGrid = isMainGridRadial();

    auto axisI = isRadialGrid ? "R" : "I";
    auto axisJ = isRadialGrid ? "θ" : "J";
    auto axisK = isRadialGrid ? "Z" : "K";

    auto makePostfix = [&]() -> QString
    {
        if ( ( cellCountI == 1 ) && ( cellCountJ > 1 ) && ( cellCountK > 1 ) )
        {
            return QString( "%1-slice %2" ).arg( axisI ).arg( startIndexI() );
        }
        else if ( ( cellCountJ == 1 ) && ( cellCountI > 1 ) && ( cellCountK > 1 ) )
        {
            return QString( "%1-slice %2" ).arg( axisJ ).arg( startIndexJ() );
        }
        else if ( ( cellCountK == 1 ) && ( cellCountI > 1 ) && ( cellCountJ > 1 ) )
        {
            return QString( "%1-slice %2" ).arg( axisK ).arg( startIndexK() );
        }
        else
        {
            QString irange = QString( "%1=%2-%3" ).arg( axisI ).arg( startIndexI() ).arg( startIndexI + cellCountI - 1 );
            QString jrange = QString( "%1=%2-%3" ).arg( axisJ ).arg( startIndexJ() ).arg( startIndexJ + cellCountJ - 1 );
            QString krange = QString( "%1=%2-%3" ).arg( axisK ).arg( startIndexK() ).arg( startIndexK + cellCountK - 1 );
            return QString( "%1 %2 %3" ).arg( irange, jrange, krange );
        }
    };

    return QString( "%1 [%2]" ).arg( RimCellFilter::fullName(), makePostfix() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellRangeFilter::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_gridIndex )
    {
        const cvf::StructGridInterface* grid = selectedGrid();

        if ( grid && grid->cellCountI() > 0 && grid->cellCountJ() > 0 && grid->cellCountK() > 0 )
        {
            cellCountI  = static_cast<int>( grid->cellCountI() );
            startIndexI = 1;

            cellCountJ  = static_cast<int>( grid->cellCountJ() );
            startIndexJ = 1;

            cellCountK  = static_cast<int>( grid->cellCountK() );
            startIndexK = 1;
        }

        filterChanged.send();
        return;
    }

    if ( changedField != &m_name )
    {
        computeAndSetValidValues();
        filterChanged.send();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellRangeFilter::computeAndSetValidValues()
{
    const cvf::StructGridInterface* grid = selectedGrid();
    if ( grid && grid->cellCountI() > 0 && grid->cellCountJ() > 0 && grid->cellCountK() > 0 )
    {
        cellCountI  = std::clamp( cellCountI.v(), 1, static_cast<int>( grid->cellCountI() ) );
        startIndexI = std::clamp( startIndexI.v(), 1, static_cast<int>( grid->cellCountI() ) );

        cellCountJ  = std::clamp( cellCountJ.v(), 1, static_cast<int>( grid->cellCountJ() ) );
        startIndexJ = std::clamp( startIndexJ.v(), 1, static_cast<int>( grid->cellCountJ() ) );

        cellCountK  = std::clamp( cellCountK.v(), 1, static_cast<int>( grid->cellCountK() ) );
        startIndexK = std::clamp( startIndexK.v(), 1, static_cast<int>( grid->cellCountK() ) );
    }
    updateIconState();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCellRangeFilter::isMainGridRadial() const
{
    auto rigGrid = dynamic_cast<const RigGridBase*>( selectedGrid() );
    if ( !rigGrid ) return false;

    auto mainGrid = rigGrid->mainGrid();
    if ( !mainGrid ) return false;

    return mainGrid->isRadial();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QString, std::vector<caf::PdmFieldHandle*>> RimCellRangeFilter::quickAccessFields()
{
    std::map<QString, std::vector<caf::PdmFieldHandle*>> fields;

    if ( cellCountI == 1 ) fields[""].push_back( &startIndexI );
    if ( cellCountJ == 1 ) fields[""].push_back( &startIndexJ );
    if ( cellCountK == 1 ) fields[""].push_back( &startIndexK );

    if ( fields.empty() )
    {
        QString groupName = "IJK Filter";
        fields[groupName].push_back( &startIndexI );
        fields[groupName].push_back( &cellCountI );
        fields[groupName].push_back( &startIndexJ );
        fields[groupName].push_back( &cellCountJ );
        fields[groupName].push_back( &startIndexK );
        fields[groupName].push_back( &cellCountK );
    }

    return fields;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellRangeFilter::setDefaultValues( int sliceDirection, int defaultSlice )
{
    const cvf::StructGridInterface* grid = selectedGrid();

    if ( !grid ) return;

    auto rimView     = firstAncestorOrThisOfType<Rim3dView>();
    auto actCellInfo = RigReservoirGridTools::activeCellInfo( rimView );

    const cvf::StructGridInterface* mainGrid = RigReservoirGridTools::mainGrid( m_srcCase );

    if ( grid == mainGrid && actCellInfo )
    {
        auto [min, max] = actCellInfo->ijkBoundingBox();

        // Adjust to Eclipse indexing
        min.x() = min.x() + 1;
        min.y() = min.y() + 1;
        min.z() = min.z() + 1;

        max.x() = max.x() + 1;
        max.y() = max.y() + 1;
        max.z() = max.z() + 1;

        startIndexI = static_cast<int>( min.x() );
        startIndexJ = static_cast<int>( min.y() );
        startIndexK = static_cast<int>( min.z() );
        cellCountI  = static_cast<int>( max.x() - min.x() + 1 );
        cellCountJ  = static_cast<int>( max.y() - min.y() + 1 );
        cellCountK  = static_cast<int>( max.z() - min.z() + 1 );
    }
    else
    {
        startIndexI = 1;
        startIndexJ = 1;
        startIndexK = 1;
        cellCountI  = static_cast<int>( grid->cellCountI() );
        cellCountJ  = static_cast<int>( grid->cellCountJ() );
        cellCountK  = static_cast<int>( grid->cellCountK() );
    }

    switch ( sliceDirection )
    {
        case 0:
            cellCountI = 1;
            if ( defaultSlice > 0 ) startIndexI = defaultSlice;
            break;
        case 1:
            cellCountJ = 1;
            if ( defaultSlice > 0 ) startIndexJ = defaultSlice;
            break;
        case 2:
            cellCountK = 1;
            if ( defaultSlice > 0 ) startIndexK = defaultSlice;
            break;
        default:
            break;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellRangeFilter::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    caf::PdmUiSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiSliderEditorAttribute*>( attribute );
    if ( !myAttr )
    {
        return;
    }

    const cvf::StructGridInterface* grid = selectedGrid();

    if ( !grid ) return;

    if ( field == &startIndexI || field == &cellCountI )
    {
        myAttr->m_minimum = 1;
        myAttr->m_maximum = static_cast<int>( grid->cellCountI() );
    }
    else if ( field == &startIndexJ || field == &cellCountJ )
    {
        myAttr->m_minimum = 1;
        myAttr->m_maximum = static_cast<int>( grid->cellCountJ() );
    }
    else if ( field == &startIndexK || field == &cellCountK )
    {
        myAttr->m_minimum = 1;
        myAttr->m_maximum = static_cast<int>( grid->cellCountK() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellRangeFilter::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimCellFilter::defineUiOrdering( uiConfigName, uiOrdering );

    const cvf::StructGridInterface* grid = selectedGrid();

    const cvf::StructGridInterface* mainGrid = RigReservoirGridTools::mainGrid( m_srcCase );

    auto rimView     = firstAncestorOrThisOfType<Rim3dView>();
    auto actCellInfo = RigReservoirGridTools::activeCellInfo( rimView );

    bool isRadialGrid = isMainGridRadial();

    if ( grid == mainGrid && actCellInfo )
    {
        auto [min, max] = actCellInfo->ijkBoundingBox();

        // Adjust to Eclipse indexing
        min.x() = min.x() + 1;
        min.y() = min.y() + 1;
        min.z() = min.z() + 1;

        max.x() = max.x() + 1;
        max.y() = max.y() + 1;
        max.z() = max.z() + 1;

        QString label;

        if ( isRadialGrid )
        {
            label = QString( "I (Radial) Active Cell Range %1 to %2, %3 cells" ).arg( min.x() ).arg( max.x() ).arg( max.x() - min.x() + 1 );
            m_labelI.uiCapability()->setUiName( label );

            label = QString( "J (Angular) Active Cell Range %1 to %2, %3 cells" ).arg( min.y() ).arg( max.y() ).arg( max.y() - min.y() + 1 );
            m_labelJ.uiCapability()->setUiName( label );

            label = QString( "K (Height) Active Cell Range %1 to %2, %3 cells" ).arg( min.z() ).arg( max.z() ).arg( max.z() - min.z() + 1 );
            m_labelK.uiCapability()->setUiName( label );
        }
        else
        {
            label = QString( "I Active Cell Range %1 to %2, %3 cells" ).arg( min.x() ).arg( max.x() ).arg( max.x() - min.x() + 1 );
            m_labelI.uiCapability()->setUiName( label );

            label = QString( "J Active Cell Range %1 to %2, %3 cells" ).arg( min.y() ).arg( max.y() ).arg( max.y() - min.y() + 1 );
            m_labelJ.uiCapability()->setUiName( label );

            label = QString( "K Active Cell Range %1 to %2, %3 cells" ).arg( min.z() ).arg( max.z() ).arg( max.z() - min.z() + 1 );
            m_labelK.uiCapability()->setUiName( label );
        }
    }
    else
    {
        if ( isRadialGrid )
        {
            m_labelI.uiCapability()->setUiName( "I (Radial)" );
            m_labelJ.uiCapability()->setUiName( "J (Angular)" );
            m_labelK.uiCapability()->setUiName( "K (Height)" );
        }
        else
        {
            m_labelI.uiCapability()->setUiName( "I" );
            m_labelJ.uiCapability()->setUiName( "J" );
            m_labelK.uiCapability()->setUiName( "K" );
        }
    }

    auto group = uiOrdering.addNewGroup( "Range Selection" );

    group->add( &m_labelI );
    group->add( &startIndexI );
    group->add( &cellCountI );

    group->add( &m_labelJ );
    group->add( &startIndexJ );
    group->add( &cellCountJ );

    group->add( &m_labelK );
    group->add( &startIndexK );
    group->add( &cellCountK );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellRangeFilter::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    RimCellFilter::defineUiTreeOrdering( uiTreeOrdering, uiConfigName );

    updateActiveState( isFilterControlled() );
    updateIconState();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellRangeFilter::updateCompundFilter( cvf::CellRangeFilter* cellRangeFilter, int gridIndex )
{
    CVF_ASSERT( cellRangeFilter );

    if ( gridIndex != m_gridIndex ) return;

    if ( filterMode() == RimCellFilter::INCLUDE )
    {
        cellRangeFilter->addCellIncludeRange( static_cast<size_t>( startIndexI ) - 1,
                                              static_cast<size_t>( startIndexJ ) - 1,
                                              static_cast<size_t>( startIndexK ) - 1,
                                              static_cast<size_t>( startIndexI ) + cellCountI - 1,
                                              static_cast<size_t>( startIndexJ ) + cellCountJ - 1,
                                              static_cast<size_t>( startIndexK ) + cellCountK - 1,
                                              propagateToSubGrids() );
    }
    else
    {
        cellRangeFilter->addCellExcludeRange( static_cast<size_t>( startIndexI ) - 1,
                                              static_cast<size_t>( startIndexJ ) - 1,
                                              static_cast<size_t>( startIndexK ) - 1,
                                              static_cast<size_t>( startIndexI ) + cellCountI - 1,
                                              static_cast<size_t>( startIndexJ ) + cellCountJ - 1,
                                              static_cast<size_t>( startIndexK ) + cellCountK - 1,
                                              propagateToSubGrids() );
    }
}
