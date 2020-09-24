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
#include "RimColorLegendCollection.h"
#include "RimEclipseResultDefinition.h"
#include "RimFractureModelTemplate.h"
#include "RimProject.h"
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
    m_faciesDefinition.uiCapability()->setUiTreeChildrenHidden( true );
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
QList<caf::PdmOptionItemInfo> RimFaciesProperties::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                          bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &m_colorLegend )
    {
        RimProject*                  project               = RimProject::current();
        RimColorLegendCollection*    colorLegendCollection = project->colorLegendCollection();
        std::vector<RimColorLegend*> colorLegends          = colorLegendCollection->allColorLegends();

        for ( RimColorLegend* colorLegend : colorLegends )
        {
            options.push_back( caf::PdmOptionItemInfo( colorLegend->colorLegendName(), colorLegend ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaciesProperties::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_filePath );
    uiOrdering.add( &m_propertiesTable );
    uiOrdering.add( &m_colorLegend );

    caf::PdmUiGroup* faciesDefinitionGroup = uiOrdering.addNewGroup( "Facies Definition" );
    m_faciesDefinition->uiOrdering( uiConfigName, *faciesDefinitionGroup );
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
        RimFractureModelTemplate* fractureModelTemplate;
        firstAncestorOrThisOfType( fractureModelTemplate );
        RicFaciesPropertiesImportTools::importFaciesPropertiesFromFile( m_filePath().path(), fractureModelTemplate );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaciesProperties::setEclipseCase( RimEclipseCase* eclipseCase )
{
    m_faciesDefinition->setEclipseCase( eclipseCase );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimColorLegend* RimFaciesProperties::colorLegend() const
{
    return m_colorLegend;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaciesProperties::setColorLegend( RimColorLegend* colorLegend )
{
    m_colorLegend = colorLegend;
}
