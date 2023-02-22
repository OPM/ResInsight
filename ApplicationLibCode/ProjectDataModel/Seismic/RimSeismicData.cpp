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

#include "RiuMainWindow.h"
#include "RiuSeismicHistogramPanel.h"

#include "cafPdmUiLineEditor.h"
#include "cafPdmUiTableViewEditor.h"

#include "cvfBoundingBox.h"

#include <QValidator>

#include <limits>
#include <tuple>

CAF_PDM_SOURCE_INIT( RimSeismicData, "SeismicData" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicData::RimSeismicData()
    : m_zStep( 0 )
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

    CAF_PDM_InitField( &m_overrideDataRange, "overrideDataRange", false, "Override Data Range" );
    CAF_PDM_InitFieldNoDefault( &m_maxAbsDataValue, "maxAbsDataValue", "Clip Value" );

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

    m_zStep = reader.depthStep();

    auto [minDataValue, maxDataValue] = reader.dataRange();
    m_maxAbsDataValue                 = std::max( std::abs( minDataValue ), std::abs( maxDataValue ) );

    reader.histogramData( m_histogramXvalues, m_histogramYvalues );

    m_worldOutline.clear();
    for ( auto& p : reader.worldCorners() )
    {
        m_worldOutline.push_back( p );
    }

    updateDataRange( false );

    reader.close();
}

std::vector<cvf::Vec3d> RimSeismicData::worldOutline() const
{
    return m_worldOutline;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicData::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( m_metadata.empty() ) updateMetaData();

    uiOrdering.add( &m_userDescription );
    uiOrdering.add( &m_filename );

    auto dsGroup = uiOrdering.addNewGroup( "Data Scaling" );
    dsGroup->add( &m_overrideDataRange );

    if ( m_overrideDataRange() )
    {
        dsGroup->add( &m_maxAbsDataValue );
    }

    auto metaGroup = uiOrdering.addNewGroup( "File Information" );

    metaGroup->add( &m_metadata );
    metaGroup->setCollapsedByDefault();

    uiOrdering.skipRemainingFields();
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
    else if ( field == &m_maxAbsDataValue )
    {
        auto myAttr = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->validator = new QDoubleValidator( 0.00001, std::numeric_limits<double>::infinity(), 100 );
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
double RimSeismicData::zStep() const
{
    return m_zStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimSeismicData::histogramXvalues() const
{
    return m_clippedHistogramXvalues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimSeismicData::histogramYvalues() const
{
    return m_clippedHistogramYvalues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicData::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                       const QVariant&            oldValue,
                                       const QVariant&            newValue )
{
    if ( ( changedField == &m_overrideDataRange ) || ( changedField == &m_maxAbsDataValue ) )
    {
        updateDataRange( true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicData::updateDataRange( bool updatePlot )
{
    m_clippedHistogramXvalues.clear();
    m_clippedHistogramYvalues.clear();

    if ( m_overrideDataRange() )
    {
        const int nVals = (int)m_histogramXvalues.size();

        for ( int i = 0; i < nVals; i++ )
        {
            double tmp = std::abs( m_histogramXvalues[i] );
            if ( tmp > m_maxAbsDataValue ) continue;
            m_clippedHistogramXvalues.push_back( m_histogramXvalues[i] );
            m_clippedHistogramYvalues.push_back( m_histogramYvalues[i] );
        }
    }
    else
    {
        for ( auto val : m_histogramXvalues )
            m_clippedHistogramXvalues.push_back( val );

        for ( auto val : m_histogramYvalues )
            m_clippedHistogramYvalues.push_back( val );
    }

    if ( updatePlot ) RiuMainWindow::instance()->seismicHistogramPanel()->showHistogram( this );
}
