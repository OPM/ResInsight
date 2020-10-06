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

#include "RimElasticPropertyScalingCollection.h"
#include "RimFractureModelTemplate.h"

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
    : changed( this )
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

    CAF_PDM_InitScriptableField( &m_showScaledProperties, "ShowScaledProperties", true, "ShowScaledProperties", "", "", "" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_scalings, "PropertyScalingCollection", "PropertyScalingCollection", "", "", "" );
    m_scalings.uiCapability()->setUiHidden( true );
    m_scalings.uiCapability()->setUiTreeHidden( true );
    m_scalings = new RimElasticPropertyScalingCollection;
    m_scalings->changed.connect( this, &RimElasticProperties::elasticPropertyScalingCollectionChanged );

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
void RimElasticProperties::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                             const QVariant&            oldValue,
                                             const QVariant&            newValue )
{
    if ( changedField == &m_showScaledProperties )
    {
        m_propertiesTable = generatePropertiesTable();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElasticProperties::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_filePath );
    uiOrdering.add( &m_showScaledProperties );
    uiOrdering.add( &m_propertiesTable );
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
                    "      <th>Immobile Fluid<br>Saturation</th>"
                    "    </tr>"
                    "  </thead>"
                    "  <tbody>" );

    std::vector<RiaDefines::CurveProperty> properties = scalableProperties();

    QString body;
    for ( auto prop : m_properties )
    {
        const QString& fieldName     = prop.second.fieldName();
        const QString& formationName = prop.second.formationName();
        const QString& faciesName    = prop.second.faciesName();

        const std::vector<double>& porosity = prop.second.porosity();

        for ( size_t i = 0; i < porosity.size(); i++ )
        {
            QString line = QString( "<tr>"
                                    "  <td>%1</td>"
                                    "  <td>%2</td>"
                                    "  <td>%3</td>"
                                    "  <td align=right>%4</td>" )
                               .arg( fieldName )
                               .arg( formationName )
                               .arg( faciesName )
                               .arg( porosity[i] );

            for ( auto property : properties )
            {
                double scale = 1.0;
                if ( m_showScaledProperties() )
                {
                    scale = getPropertyScaling( formationName, faciesName, property );
                }
                double value = prop.second.getValue( property, i, scale );

                QString propertyElement = QString( "<td align=right>%1</td>" ).arg( value );
                line.append( propertyElement );
            }

            line.append( "</tr>" );

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
        RimFractureModelTemplate* fractureModelTemplate;
        firstAncestorOrThisOfType( fractureModelTemplate );
        RicElasticPropertiesImportTools::importElasticPropertiesFromFile( m_filePath().path(), fractureModelTemplate );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimElasticPropertyScalingCollection* RimElasticProperties::scalingCollection()
{
    return m_scalings.value();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaDefines::CurveProperty> RimElasticProperties::scalableProperties()
{
    std::vector<RiaDefines::CurveProperty> properties = {
        RiaDefines::CurveProperty::YOUNGS_MODULUS,
        RiaDefines::CurveProperty::POISSONS_RATIO,
        RiaDefines::CurveProperty::K_IC,
        RiaDefines::CurveProperty::PROPPANT_EMBEDMENT,
        RiaDefines::CurveProperty::BIOT_COEFFICIENT,
        RiaDefines::CurveProperty::K0,
        RiaDefines::CurveProperty::FLUID_LOSS_COEFFICIENT,
        RiaDefines::CurveProperty::SPURT_LOSS,
        RiaDefines::CurveProperty::IMMOBILE_FLUID_SATURATION,
    };

    return properties;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimElasticProperties::isScalableProperty( RiaDefines::CurveProperty property )
{
    std::vector<RiaDefines::CurveProperty> properties = scalableProperties();
    return std::find( properties.begin(), properties.end(), property ) != properties.end();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimElasticProperties::getPropertyScaling( const QString&            formationName,
                                                 const QString&            faciesName,
                                                 RiaDefines::CurveProperty property ) const
{
    if ( m_scalings )
    {
        return m_scalings->getScaling( formationName, faciesName, property );
    }

    return 1.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElasticProperties::elasticPropertyScalingCollectionChanged( const caf::SignalEmitter* emitter )
{
    m_propertiesTable = generatePropertiesTable();
    changed.send();
}
