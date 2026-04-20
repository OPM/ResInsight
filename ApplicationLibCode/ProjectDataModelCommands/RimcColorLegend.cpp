/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026- Equinor ASA
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

#include "RimcColorLegend.h"

#include "RimCase.h"
#include "RimColorLegend.h"
#include "RimColorLegendCollection.h"
#include "RimColorLegendItem.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapabilityCvfColor3.h"

#include "cvfColor3.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimColorLegendCollection, RimcColorLegendCollection_createColorLegend, "CreateColorLegend" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcColorLegendCollection_createColorLegend::RimcColorLegendCollection_createColorLegend( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Create Color Legend", "", "", "Create a new custom color legend" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_legendName, "Name", "Name" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcColorLegendCollection_createColorLegend::execute()
{
    auto collection = self<RimColorLegendCollection>();
    if ( !collection ) return std::unexpected( "No color legend collection found" );

    auto* legend = new RimColorLegend();
    legend->setColorLegendName( m_legendName() );

    collection->appendCustomColorLegend( legend );
    collection->updateConnectedEditors();

    return legend;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcColorLegendCollection_createColorLegend::classKeywordReturnedType() const
{
    return RimColorLegend::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimColorLegend, RimcColorLegend_addColorLegendItem, "AddColorLegendItem" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcColorLegend_addColorLegendItem::RimcColorLegend_addColorLegendItem( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Add Color Legend Item", "", "", "Append a category item with a name and RGB color" );

    CAF_PDM_InitScriptableField( &m_categoryValue, "CategoryValue", 0, "Category Value" );
    CAF_PDM_InitScriptableField( &m_categoryName, "CategoryName", QString(), "Category Name" );
    CAF_PDM_InitScriptableField( &m_color, "Color", cvf::Color3f( cvf::Color3f::ORANGE ), "Color" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcColorLegend_addColorLegendItem::execute()
{
    auto legend = self<RimColorLegend>();
    if ( !legend ) return std::unexpected( "No color legend found" );

    auto* item = new RimColorLegendItem();
    item->setValues( m_categoryName(), m_categoryValue(), m_color() );
    legend->appendColorLegendItem( item );

    return item;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcColorLegend_addColorLegendItem::classKeywordReturnedType() const
{
    return RimColorLegendItem::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimColorLegendCollection,
                                   RimcColorLegendCollection_setDefaultColorLegendForResult,
                                   "SetDefaultColorLegendForResult" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcColorLegendCollection_setDefaultColorLegendForResult::RimcColorLegendCollection_setDefaultColorLegendForResult( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Set Default Color Legend For Result", "", "", "Bind a color legend to a (case, resultName) pair" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_case, "Case", "Case" );
    CAF_PDM_InitScriptableField( &m_resultName, "ResultName", QString(), "Result Name" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_colorLegend, "ColorLegend", "Color Legend" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcColorLegendCollection_setDefaultColorLegendForResult::execute()
{
    auto collection = self<RimColorLegendCollection>();
    if ( !collection ) return std::unexpected( "No color legend collection found" );

    if ( !m_case() ) return std::unexpected( "No case provided" );
    if ( !m_colorLegend() ) return std::unexpected( "No color legend provided" );

    collection->setDefaultColorLegendForResult( m_case(), m_resultName(), m_colorLegend() );

    return nullptr;
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimColorLegendCollection, RimcColorLegendCollection_deleteColorLegend, "DeleteColorLegend" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcColorLegendCollection_deleteColorLegend::RimcColorLegendCollection_deleteColorLegend( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Delete Color Legend", "", "", "Remove the color legend bound to a (case, resultName) pair" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_case, "Case", "Case" );
    CAF_PDM_InitScriptableField( &m_resultName, "ResultName", QString(), "Result Name" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcColorLegendCollection_deleteColorLegend::execute()
{
    auto collection = self<RimColorLegendCollection>();
    if ( !collection ) return std::unexpected( "No color legend collection found" );

    if ( !m_case() ) return std::unexpected( "No case provided" );

    collection->deleteColorLegend( m_case(), m_resultName() );
    collection->updateConnectedEditors();

    return nullptr;
}
