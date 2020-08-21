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

#include "RimElasticProperties.h"

#include "RimFractureModel.h"

#include "RicElasticPropertiesImportTools.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiTextEditor.h"

CAF_PDM_SOURCE_INIT( RimElasticProperties, "ElasticProperties" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimElasticProperties::RimElasticProperties()
{
    CAF_PDM_InitScriptableObject( "RimElasticProperties", "", "", "" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_filePath, "FilePath", "File Path", "", "", "" );
    m_filePath.uiCapability()->setUiReadOnly( true );
    m_filePath.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitScriptableFieldNoDefault( &m_propertiesTable, "PropertiesTable", "Properties Table", "", "", "" );
    m_propertiesTable.uiCapability()->setUiEditorTypeName( caf::PdmUiTextEditor::uiEditorTypeName() );
    m_propertiesTable.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_propertiesTable.uiCapability()->setUiReadOnly( true );
    m_propertiesTable.xmlCapability()->disableIO();

    setUiName( "Elastic Properties" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimElasticProperties::~RimElasticProperties()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimElasticProperties::filePath() const
{
    return m_filePath.v().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElasticProperties::setFilePath( const QString& filePath )
{
    m_filePath = filePath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElasticProperties::setPropertiesForFacies( FaciesKey& key, const RigElasticProperties& properties )
{
    m_properties.insert( std::pair<FaciesKey, RigElasticProperties>( key, properties ) );
    m_propertiesTable = generatePropertiesTable();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimElasticProperties::hasPropertiesForFacies( FaciesKey& key ) const
{
    return m_properties.find( key ) != m_properties.end();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigElasticProperties& RimElasticProperties::propertiesForFacies( FaciesKey& key ) const
{
    assert( hasPropertiesForFacies( key ) );
    return m_properties.find( key )->second;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElasticProperties::defineEditorAttribute( const caf::PdmFieldHandle* field,
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
QString RimElasticProperties::generatePropertiesTable()
{
    QString header( "<table border=1>"
                    "  <thead>"
                    "    <tr bgcolor=lightblue>"
                    "      <th>Field</th>"
                    "      <th>Formation</th>"
                    "      <th>Facies</th>"
                    "      <th>Porosity</th>"
                    "      <th>Young's<br>Modulus</th>"
                    "      <th>Poisson's<br>Ratio</th>"
                    "      <th>K-Ic</th>"
                    "      <th>Proppant<br>Embedment</th>"
                    "      <th>Biot<br>Coefficient</th>"
                    "      <th>k0</th>"
                    "      <th>Fluid Loss<br>Coefficient</th>"
                    "      <th>Spurt Loss</th>"
                    "    </tr>"
                    "  </thead>"
                    "  <tbody>" );

    QString body;
    for ( auto prop : m_properties )
    {
        const QString&             fieldName            = prop.second.fieldName();
        const std::vector<double>& porosity             = prop.second.porosity();
        const std::vector<double>& youngsModulus        = prop.second.youngsModulus();
        const std::vector<double>& poissonsRatio        = prop.second.poissonsRatio();
        const std::vector<double>& K_Ic                 = prop.second.K_Ic();
        const std::vector<double>& proppantEmbedment    = prop.second.proppantEmbedment();
        const std::vector<double>& biotCoefficient      = prop.second.biotCoefficient();
        const std::vector<double>& k0                   = prop.second.k0();
        const std::vector<double>& fluidLossCoefficient = prop.second.fluidLossCoefficient();
        const std::vector<double>& spurtLoss            = prop.second.spurtLoss();

        for ( size_t i = 0; i < porosity.size(); i++ )
        {
            QString format( "<tr>"
                            "  <td>%1</td>"
                            "  <td>%2</td>"
                            "  <td>%3</td>"
                            "  <td align=right>%4</td>"
                            "  <td align=right>%5</td>"
                            "  <td align=right>%6</td>"
                            "  <td align=right>%7</td>"
                            "  <td align=right>%8</td>"
                            "  <td align=right>%9</td>"
                            "  <td align=right>%10</td>"
                            "  <td align=right>%11</td>"
                            "  <td align=right>%12</td>"
                            "</tr>" );

            QString line = format.arg( fieldName )
                               .arg( prop.second.formationName() )
                               .arg( prop.second.faciesName() )
                               .arg( porosity[i] )
                               .arg( youngsModulus[i] )
                               .arg( poissonsRatio[i] )
                               .arg( K_Ic[i] )
                               .arg( proppantEmbedment[i] )
                               .arg( biotCoefficient[i] )
                               .arg( k0[i] )
                               .arg( fluidLossCoefficient[i] )
                               .arg( spurtLoss[i] );

            body.append( line );
        }
    }

    QString footer( "</tbody></table>" );

    return header + body + footer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElasticProperties::loadDataAndUpdate()
{
    if ( !m_filePath().path().isEmpty() )
    {
        RimFractureModel* fractureModel;
        firstAncestorOrThisOfType( fractureModel );
        RicElasticPropertiesImportTools::importElasticPropertiesFromFile( m_filePath().path(), fractureModel );
    }
}
