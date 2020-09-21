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

#include "RimFaciesProperties.h"

#include "RimColorLegend.h"
#include "RimEclipseResultDefinition.h"
#include "RimFractureModel.h"
#include "RimRegularLegendConfig.h"

#include "RicFaciesPropertiesImportTools.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiTextEditor.h"

CAF_PDM_SOURCE_INIT( RimFaciesProperties, "FaciesProperties" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaciesProperties::RimFaciesProperties()
{
    CAF_PDM_InitScriptableObject( "RimFaciesProperties", "", "", "" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_filePath, "FilePath", "File Path", "", "", "" );
    m_filePath.uiCapability()->setUiReadOnly( true );
    m_filePath.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitScriptableFieldNoDefault( &m_propertiesTable, "PropertiesTable", "Properties Table", "", "", "" );
    m_propertiesTable.uiCapability()->setUiEditorTypeName( caf::PdmUiTextEditor::uiEditorTypeName() );
    m_propertiesTable.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_propertiesTable.uiCapability()->setUiReadOnly( true );
    m_propertiesTable.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_faciesDefinition, "FaciesDefinition", "", "", "", "" );
    m_faciesDefinition.uiCapability()->setUiHidden( true );
    // m_faciesDefinition.uiCapability()->setUiTreeChildrenHidden( true );
    m_faciesDefinition = new RimEclipseResultDefinition;
    m_faciesDefinition->findField( "MResultType" )->uiCapability()->setUiName( "Facies Definiton" );

    CAF_PDM_InitFieldNoDefault( &m_colorLegend, "ColorLegend", "Colors", "", "", "" );
    m_colorLegend = RimRegularLegendConfig::mapToColorLegend( RimRegularLegendConfig::ColorRangesType::NORMAL );

    setUiName( "Facies Properties" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaciesProperties::~RimFaciesProperties()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaciesProperties::filePath() const
{
    return m_filePath.v().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaciesProperties::setFilePath( const QString& filePath )
{
    m_filePath = filePath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaciesProperties::setFaciesCodeName( int code, const QString& name )
{
    m_faciesCodeNames[code] = name;
    m_propertiesTable       = generatePropertiesTable();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaciesProperties::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_filePath );
    uiOrdering.add( &m_faciesDefinition );
    uiOrdering.add( &m_propertiesTable );
    uiOrdering.add( &m_colorLegend );
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaciesProperties::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                 QString                    uiConfigName,
                                                 caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_propertiesTable )
    {
        auto myAttr = dynamic_cast<caf::PdmUiTextEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->wrapMode = caf::PdmUiTextEditorAttribute::NoWrap;
            myAttr->textMode = caf::PdmUiTextEditorAttribute::HTML;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaciesProperties::generatePropertiesTable()
{
    QString header( "<table border=1>"
                    "  <thead>"
                    "    <tr bgcolor=lightblue>"
                    "      <th>Name</th>"
                    "      <th>Index</th>"
                    "    </tr>"
                    "  </thead>"
                    "  <tbody>" );

    QString body;
    for ( auto prop : m_faciesCodeNames )
    {
        int            index = prop.first;
        const QString& name  = prop.second;

        QString format( "<tr>"
                        "  <td>%1</td>"
                        "  <td>%2</td>"
                        "</tr>" );

        QString line = format.arg( name ).arg( index );
        body.append( line );
    }

    QString footer( "</tbody></table>" );

    return header + body + footer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaciesProperties::loadDataAndUpdate()
{
    if ( !m_filePath().path().isEmpty() )
    {
        RimFractureModel* fractureModel;
        firstAncestorOrThisOfType( fractureModel );
        RicFaciesPropertiesImportTools::importFaciesPropertiesFromFile( m_filePath().path(), fractureModel );
    }
}
