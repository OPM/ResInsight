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

#include "RimSeismicDifferenceData.h"

#include "RiaLogging.h"

#include "RifOpenVDSReader.h"
#include "RifSeismicZGYReader.h"

#include "RimRegularLegendConfig.h"
#include "RimSeismicAlphaMapper.h"
#include "RimStringParameter.h"

#include "RiuMainWindow.h"
#include "RiuSeismicHistogramPanel.h"

#include <zgyaccess/seismicslice.h>

#include "cafPdmUiLineEditor.h"
#include "cafPdmUiTableViewEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include "cvfBoundingBox.h"

#include <QFile>
#include <QFileInfo>
#include <QValidator>

#include <algorithm>
#include <cmath>
#include <limits>
#include <tuple>

CAF_PDM_SOURCE_INIT( RimSeismicDifferenceData, "SeismicDifferenceData" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicDifferenceData::RimSeismicDifferenceData()
    : m_fileDataRange( 0, 0 )
    , m_activeDataRange( 0, 0 )
    , m_inputDataOK( false )
{
    CAF_PDM_InitObject( "SeismicDifferenceData", ":/Seismic16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_userDescription, "SeismicUserDecription", "Name" );

    CAF_PDM_InitFieldNoDefault( &m_legendConfig, "LegendDefinition", "Color Legend" );
    m_legendConfig = new RimRegularLegendConfig();
    m_legendConfig.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitField( &m_overrideDataRange, "overrideDataRange", false, "Override Data Range" );
    CAF_PDM_InitField( &m_userClipValue, "userClipValue", 0.0, "Clip Value" );

    CAF_PDM_InitFieldNoDefault( &m_seismicData1, "SeismicData1", "Seismic Data 1" );
    CAF_PDM_InitFieldNoDefault( &m_seismicData2, "SeismicData2", "Seismic Data 2" );

    setDeletable( true );

    m_alphaValueMapper = std::make_shared<RimSeismicAlphaMapper>();

    initColorLegend();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicDifferenceData::~RimSeismicDifferenceData()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicDifferenceData::setInputData( RimSeismicDataInterface* data1, RimSeismicDataInterface* data2 )
{
    m_seismicData1 = data1;
    m_seismicData2 = data2;

    updateMetaData();
    updateDataRange( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicDifferenceData::initAfterRead()
{
    updateMetaData();
    updateDataRange( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSeismicDifferenceData::isInputDataOK() const
{
    return m_inputDataOK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimSeismicDifferenceData::userDescription() const
{
    return m_userDescription().toStdString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicDifferenceData::setUserDescription( QString description )
{
    m_userDescription = description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSeismicDifferenceData::userDescriptionField()
{
    return &m_userDescription;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RimSeismicDifferenceData::worldOutline() const
{
    if ( !isInputDataOK() ) return {};
    return m_seismicData1->worldOutline();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicDifferenceData::updateMetaData()
{
    m_inputDataOK = false;
    if ( ( m_seismicData1 == nullptr ) || ( m_seismicData2 == nullptr ) ) return;

    // todo - check that data use same grid - use the same method to select valid entries for seismic input 2

    // todo - generate histogram
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicDifferenceData::initColorLegend()
{
    m_legendConfig->setColorLegend( RimRegularLegendConfig::mapToColorLegend( RimRegularLegendConfig::ColorRangesType::BLUE_WHITE_RED ) );
    m_legendConfig->setMappingMode( RimRegularLegendConfig::MappingType::LINEAR_CONTINUOUS );
    m_legendConfig->setRangeMode( RimLegendConfig::RangeModeType::USER_DEFINED );
    m_legendConfig->setCenterLegendAroundZero( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicDifferenceData::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto genGroup = uiOrdering.addNewGroup( "General" );
    genGroup->add( &m_userDescription );

    auto diffGroup = uiOrdering.addNewGroup( "Difference Data" );
    diffGroup->add( &m_seismicData1 );
    diffGroup->add( &m_seismicData2 );

    auto cmGroup = uiOrdering.addNewGroup( "Color Mapping" );
    m_legendConfig->defineUiOrderingColorOnly( cmGroup );
    cmGroup->add( &m_overrideDataRange );
    if ( m_overrideDataRange() )
    {
        cmGroup->add( &m_userClipValue );
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicDifferenceData::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= "" */ )
{
    uiTreeOrdering.skipRemainingChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicDifferenceData::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_userClipValue )
    {
        auto myAttr = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->validator = new QDoubleValidator( 0.00001, std::numeric_limits<double>::infinity(), 6 );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox* RimSeismicDifferenceData::boundingBox() const
{
    return m_boundingBox.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimSeismicDifferenceData::zMin() const
{
    if ( !isInputDataOK() ) return 0.0;
    return std::abs( m_boundingBox->max().z() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimSeismicDifferenceData::zMax() const
{
    if ( !isInputDataOK() ) return 0.0;
    return std::abs( m_boundingBox->min().z() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimSeismicDifferenceData::zStep() const
{
    if ( !isInputDataOK() ) return 1.0;
    return m_seismicData1->zStep();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSeismicDifferenceData::inlineMin() const
{
    if ( !isInputDataOK() ) return 0;
    return m_seismicData1->inlineMin();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSeismicDifferenceData::inlineMax() const
{
    if ( !isInputDataOK() ) return 0;
    return m_seismicData1->inlineMax();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSeismicDifferenceData::inlineStep() const
{
    if ( !isInputDataOK() ) return 1;
    return m_seismicData1->inlineStep();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSeismicDifferenceData::xlineMin() const
{
    if ( !isInputDataOK() ) return 0;
    return m_seismicData1->xlineMin();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSeismicDifferenceData::xlineMax() const
{
    if ( !isInputDataOK() ) return 0;
    return m_seismicData1->xlineMax();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSeismicDifferenceData::xlineStep() const
{
    if ( !isInputDataOK() ) return 1;
    return m_seismicData1->xlineStep();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimSeismicDifferenceData::histogramXvalues() const
{
    return m_clippedHistogramXvalues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimSeismicDifferenceData::histogramYvalues() const
{
    return m_clippedHistogramYvalues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimSeismicDifferenceData::alphaValues() const
{
    return m_clippedAlphaValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicDifferenceData::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( ( changedField == &m_overrideDataRange ) || ( changedField == &m_userClipValue ) )
    {
        updateDataRange( true );
    }
    else if ( ( changedField == &m_seismicData1 ) || ( changedField == &m_seismicData2 ) )
    {
        updateMetaData();
        updateDataRange( true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicDifferenceData::updateDataRange( bool updatePlot )
{
    m_clippedHistogramXvalues.clear();
    m_clippedHistogramYvalues.clear();
    m_clippedAlphaValues.clear();

    if ( !isInputDataOK() )
    {
        if ( updatePlot ) RiuMainWindow::instance()->seismicHistogramPanel()->showHistogram( (RimSeismicDataInterface*)nullptr );
        return;
    }

    double clipValue = m_userClipValue;

    if ( m_overrideDataRange() )
    {
        m_activeDataRange = std::make_pair( -m_userClipValue, m_userClipValue );
    }
    else
    {
        m_activeDataRange = std::make_pair( m_fileDataRange.first, m_fileDataRange.second );
        clipValue         = m_fileDataRange.second;
    }

    const int nVals = (int)m_histogramXvalues.size();

    for ( int i = 0; i < nVals; i++ )
    {
        double tmp = std::abs( m_histogramXvalues[i] );
        if ( tmp > clipValue ) continue;
        m_clippedHistogramXvalues.push_back( m_histogramXvalues[i] );
        m_clippedHistogramYvalues.push_back( m_histogramYvalues[i] );
    }

    double maxRawValue = *std::max_element( m_clippedHistogramYvalues.begin(), m_clippedHistogramYvalues.end() );
    for ( auto val : m_clippedHistogramYvalues )
    {
        m_clippedAlphaValues.push_back( 1.0 - std::clamp( val / maxRawValue, 0.0, 1.0 ) );
    }

    m_alphaValueMapper->setDataRangeAndAlphas( m_activeDataRange.first, m_activeDataRange.second, m_clippedAlphaValues );
    m_legendConfig->setUserDefinedRange( m_activeDataRange.first, m_activeDataRange.second );

    if ( updatePlot ) RiuMainWindow::instance()->seismicHistogramPanel()->showHistogram( (RimSeismicDataInterface*)this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimSeismicDifferenceData::convertToWorldCoords( int iLine, int xLine, double depth )
{
    if ( !isInputDataOK() ) return { 0, 0, 0 };

    return m_seismicData1->convertToWorldCoords( iLine, xLine, depth );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<int, int> RimSeismicDifferenceData::convertToInlineXline( cvf::Vec3d worldCoords )
{
    if ( !isInputDataOK() ) return { 0, 0 };

    return m_seismicData1->convertToInlineXline( worldCoords );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RimSeismicDifferenceData::dataRangeMinMax() const
{
    if ( !isInputDataOK() ) return { 0, 0 };
    return m_activeDataRange;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<ZGYAccess::SeismicSliceData>
    RimSeismicDifferenceData::sliceData( RiaDefines::SeismicSliceDirection direction, int sliceNumber, double zMin, double zMax )
{
    if ( !isInputDataOK() ) return nullptr;

    auto data1 = m_seismicData1->sliceData( direction, sliceNumber, zMin, zMax );
    auto data2 = m_seismicData2->sliceData( direction, sliceNumber, zMin, zMax );

    return data1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig* RimSeismicDifferenceData::legendConfig() const
{
    return m_legendConfig();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicAlphaMapper* RimSeismicDifferenceData::alphaValueMapper() const
{
    return m_alphaValueMapper.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<ZGYAccess::SeismicSliceData>
    RimSeismicDifferenceData::sliceData( double worldX1, double worldY1, double worldX2, double worldY2, double zMin, double zMax )
{
    if ( !isInputDataOK() ) return nullptr;

    auto data1 = m_seismicData1->sliceData( worldX1, worldY1, worldX2, worldY2, zMin, zMax );
    auto data2 = m_seismicData1->sliceData( worldX1, worldY1, worldX2, worldY2, zMin, zMax );

    return data1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
float RimSeismicDifferenceData::valueAt( cvf::Vec3d worldCoord )
{
    if ( !isInputDataOK() ) return std::nanf( "" );

    float val1 = m_seismicData1->valueAt( worldCoord );
    float val2 = m_seismicData2->valueAt( worldCoord );

    return val1 - val2;
}
