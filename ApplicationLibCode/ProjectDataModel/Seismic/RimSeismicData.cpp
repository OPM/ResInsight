/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RimSeismicData.h"

#include "RifSeismicZGYReader.h"
#include "RimStringParameter.h"

#include "cafPdmUiTableViewEditor.h"

#include "cvfBoundingBox.h"

#include <tuple>

CAF_PDM_SOURCE_INIT( RimSeismicData, "SeismicData" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicData::RimSeismicData()
{
    CAF_PDM_InitObject( "SeismicData", ":/Seismic16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_userDescription, "SeismicUserDecription", "Name" );

    CAF_PDM_InitFieldNoDefault( &m_filename, "SeismicFilePath", "File" );
    m_filename.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_metadata, "Metadata", "Metadata" );
    m_metadata.uiCapability()->setUiEditorTypeName( caf::PdmUiTableViewEditor::uiEditorTypeName() );
    m_metadata.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_metadata.uiCapability()->setCustomContextMenuEnabled( true );
    m_metadata.uiCapability()->setUiTreeChildrenHidden( true );
    m_metadata.uiCapability()->setUiTreeHidden( true );
    m_metadata.uiCapability()->setUiReadOnly( true );
    m_metadata.xmlCapability()->disableIO();

    setDeletable( true );

    m_boundingBox = std::make_shared<cvf::BoundingBox>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicData::~RimSeismicData()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicData::initAfterRead()
{
    updateMetaData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicData::setFileName( QString filename )
{
    m_filename = filename;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSeismicData::fileName() const
{
    return m_filename;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSeismicData::userDescription()
{
    return m_userDescription;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicData::setUserDescription( QString description )
{
    m_userDescription = description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSeismicData::userDescriptionField()
{
    return &m_userDescription;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicData::updateMetaData()
{
    m_metadata.deleteChildren();

    RifSeismicZGYReader reader;

    if ( !reader.open( m_filename ) ) return;

    auto metadata = reader.metaData();

    for ( auto [name, value] : metadata )
    {
        auto param = new RimStringParameter();
        param->setLabel( name );
        param->setValue( value );
        m_metadata.push_back( param );
    }

    m_boundingBox->reset();
    m_boundingBox->add( reader.boundingBox() );

    reader.histogramData( m_histogramXvalues, m_histogramYvalues );

    reader.close();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicData::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( m_metadata.empty() ) updateMetaData();

    caf::PdmObject::defineUiOrdering( uiConfigName, uiOrdering );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicData::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                            QString                    uiConfigName,
                                            caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_metadata )
    {
        auto tvAttribute = dynamic_cast<caf::PdmUiTableViewEditorAttribute*>( attribute );
        if ( tvAttribute )
        {
            tvAttribute->resizePolicy              = caf::PdmUiTableViewEditorAttribute::RESIZE_TO_FILL_CONTAINER;
            tvAttribute->alwaysEnforceResizePolicy = true;
            tvAttribute->minimumHeight             = 400;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox* RimSeismicData::boundingBox() const
{
    return m_boundingBox.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimSeismicData::zMin() const
{
    return m_boundingBox.get()->min().z();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimSeismicData::zMax() const
{
    return m_boundingBox.get()->max().z();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimSeismicData::histogramXvalues() const
{
    return m_histogramXvalues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimSeismicData::histogramYvalues() const
{
    return m_histogramYvalues;
}
