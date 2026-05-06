/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RimcCombinedFilter.h"

#include "RiaDefines.h"

#include "RicEclipsePropertyFilterFeatureImpl.h"

#include "RimCellRangeFilter.h"
#include "RimCombinedFilter.h"
#include "RimEclipsePropertyFilter.h"
#include "RimEclipseResultDefinition.h"

#include "Rim3dView.h"

#include "cafAppEnum.h"
#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimCombinedFilter, RimcCombinedFilter_addPropertyFilter, "AddPropertyFilter" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcCombinedFilter_addPropertyFilter::RimcCombinedFilter_addPropertyFilter( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Add Property Filter", "", "", "Add a new property filter as a child of this combined filter" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_resultVariable, "ResultVariable", "Result Variable" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_resultType, "ResultType", "Result Type" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcCombinedFilter_addPropertyFilter::execute()
{
    auto* combined = self<RimCombinedFilter>();
    if ( !combined ) return std::unexpected( QString( "Self is not a RimCombinedFilter" ) );

    RimEclipsePropertyFilter* pf = RicEclipsePropertyFilterFeatureImpl::addPropertyFilterToCombinedFilter( combined );
    if ( !pf ) return std::unexpected( QString( "Failed to create property filter" ) );

    const bool hasType = !m_resultType().isEmpty();
    const bool hasVar  = !m_resultVariable().isEmpty();

    if ( hasType )
    {
        const auto cat = caf::AppEnum<RiaDefines::ResultCatType>::fromText( m_resultType() );
        pf->resultDefinition()->setResultType( cat );
    }
    if ( hasVar )
    {
        pf->resultDefinition()->setResultVariable( m_resultVariable() );
    }
    if ( hasType || hasVar )
    {
        pf->resultDefinition()->loadResult();
        pf->setToDefaultValues();
        pf->updateFilterName();
    }

    pf->updateConnectedEditors();

    return pf;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcCombinedFilter_addPropertyFilter::classKeywordReturnedType() const
{
    return RimEclipsePropertyFilter::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimCombinedFilter, RimcCombinedFilter_addRangeFilter, "AddRangeFilter" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcCombinedFilter_addRangeFilter::RimcCombinedFilter_addRangeFilter( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Add Range Filter", "", "", "Add a new IJK range filter as a child of this combined filter" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_name, "Name", "Name" );
    CAF_PDM_InitScriptableField( &m_startI, "StartI", -1, "Start I" );
    CAF_PDM_InitScriptableField( &m_startJ, "StartJ", -1, "Start J" );
    CAF_PDM_InitScriptableField( &m_startK, "StartK", -1, "Start K" );
    CAF_PDM_InitScriptableField( &m_cellCountI, "CellCountI", -1, "Cell Count I" );
    CAF_PDM_InitScriptableField( &m_cellCountJ, "CellCountJ", -1, "Cell Count J" );
    CAF_PDM_InitScriptableField( &m_cellCountK, "CellCountK", -1, "Cell Count K" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcCombinedFilter_addRangeFilter::execute()
{
    auto* combined = self<RimCombinedFilter>();
    if ( !combined ) return std::unexpected( QString( "Self is not a RimCombinedFilter" ) );

    auto* rf = combined->addNewFilter<RimCellRangeFilter>(
        []( RimCellRangeFilter* f )
        {
            f->setGridIndex( 0 );
            f->setDefaultValues( -1, -1 );
        } );

    if ( !rf ) return std::unexpected( QString( "Failed to create range filter" ) );

    if ( !m_name().isEmpty() ) rf->setName( m_name() );

    if ( m_startI() > 0 ) rf->startIndexI = m_startI();
    if ( m_startJ() > 0 ) rf->startIndexJ = m_startJ();
    if ( m_startK() > 0 ) rf->startIndexK = m_startK();
    if ( m_cellCountI() > 0 ) rf->cellCountI = m_cellCountI();
    if ( m_cellCountJ() > 0 ) rf->cellCountJ = m_cellCountJ();
    if ( m_cellCountK() > 0 ) rf->cellCountK = m_cellCountK();

    rf->updateConnectedEditors();

    return rf;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcCombinedFilter_addRangeFilter::classKeywordReturnedType() const
{
    return RimCellRangeFilter::classKeywordStatic();
}
