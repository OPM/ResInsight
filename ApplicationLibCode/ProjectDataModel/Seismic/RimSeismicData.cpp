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

#include "RiaLogging.h"

#include "RifSeismicZGYReader.h"

#include "RimRegularLegendConfig.h"
#include "RimSeismicAlphaMapper.h"
#include "RimStringParameter.h"

#include "RiuMainWindow.h"
#include "RiuSeismicHistogramPanel.h"

#include <seismicslice.h>

#include "cafPdmUiLineEditor.h"
#include "cafPdmUiTableViewEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include "cvfBoundingBox.h"

#include <QValidator>

#include <algorithm>
#include <cmath>
#include <limits>
#include <tuple>

CAF_PDM_SOURCE_INIT( RimSeismicData, "SeismicData" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicData::RimSeismicData()
    : m_zStep( 0 )
    , m_filereader( nullptr )
    , m_nErrorsLogged( 0 )
    , m_fileDataRange( 0, 0 )
    , m_activeDataRange( 0, 0 )
{
    CAF_PDM_InitObject( "SeismicData", ":/Seismic16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_userDescription, "SeismicUserDecription", "Name" );

    CAF_PDM_InitFieldNoDefault( &m_filename, "SeismicFilePath", "File" );
    m_filename.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_legendConfig, "LegendDefinition", "Color Legend" );
    m_legendConfig = new RimRegularLegendConfig();
    m_legendConfig.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_metadata, "Metadata", "Metadata" );
    m_metadata.uiCapability()->setUiEditorTypeName( caf::PdmUiTableViewEditor::uiEditorTypeName() );
    m_metadata.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_metadata.uiCapability()->setUiTreeChildrenHidden( true );
    m_metadata.uiCapability()->setUiTreeHidden( true );
    m_metadata.uiCapability()->setUiReadOnly( true );
    m_metadata.xmlCapability()->disableIO();

    CAF_PDM_InitField( &m_overrideDataRange, "overrideDataRange", false, "Override Data Range" );
    CAF_PDM_InitField( &m_userClipValue, "userClipValue", 0.0, "Clip Value" );

    setDeletable( true );

    m_boundingBox      = std::make_shared<cvf::BoundingBox>();
    m_alphaValueMapper = std::make_shared<RimSeismicAlphaMapper>();

    initColorLegend();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicData::~RimSeismicData()
{
    if ( m_filereader != nullptr ) m_filereader->close();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSeismicData::openFileIfNotOpen()
{
    if ( m_filereader == nullptr )
    {
        m_filereader = std::make_shared<RifSeismicZGYReader>();
    }

    if ( m_filereader->isOpen() ) return true;

    QString filename = m_filename().path();

    if ( filename.isEmpty() ) return false;

    if ( QFile::exists( filename ) )
    {
        if ( !m_filereader->open( filename ) )
        {
            logError( "Unable to open seismic file : " + filename );
            m_filereader.reset();
            return false;
        }
        if ( !m_filereader->isValid() )
        {
            logError( "Seismic file has invalid header values. Cannot import file: " + filename );
            m_filereader->close();
            m_filereader.reset();
            return false;
        }
    }
    else
    {
        logError( "Seismic file not found: " + filename );
        return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicData::logError( QString msg )
{
    if ( m_nErrorsLogged < 4 ) RiaLogging::error( msg );
    m_nErrorsLogged++;
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
    if ( filename != m_filename().path() )
    {
        if ( m_filereader != nullptr ) m_filereader->close();
        m_nErrorsLogged = 0;
        m_filename      = filename;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSeismicData::fileName() const
{
    return m_filename().path();
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
    m_boundingBox->reset();
    m_worldOutline.clear();

    if ( !openFileIfNotOpen() ) return;

    auto metadata = m_filereader->metaData();

    for ( auto& [name, value] : metadata )
    {
        auto param = new RimStringParameter();
        param->setLabel( name );
        param->setValue( value );
        m_metadata.push_back( param );
    }

    m_boundingBox->add( m_filereader->boundingBox() );

    m_zStep = m_filereader->zStep();

    auto [minDataValue, maxDataValue] = m_filereader->dataRange();
    double maxAbsDataValue            = std::max( std::abs( minDataValue ), std::abs( maxDataValue ) );

    if ( m_userClipValue <= 0.0 ) m_userClipValue = maxAbsDataValue;

    m_userClipValue = std::clamp( m_userClipValue(), 0.0, maxAbsDataValue );

    m_filereader->histogramData( m_histogramXvalues, m_histogramYvalues );

    for ( auto& p : m_filereader->worldCorners() )
    {
        m_worldOutline.push_back( p );
    }

    m_inlineInfo = m_filereader->inlineMinMaxStep();
    m_xlineInfo  = m_filereader->xlineMinMaxStep();

    m_fileDataRange = m_filereader->dataRange();

    updateDataRange( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RimSeismicData::worldOutline() const
{
    return m_worldOutline;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicData::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( m_metadata.empty() )
    {
        updateMetaData();
    }

    auto genGroup = uiOrdering.addNewGroup( "General" );
    genGroup->add( &m_userDescription );
    genGroup->add( &m_filename );

    auto cmGroup = uiOrdering.addNewGroup( "Color Mapping" );
    m_legendConfig->defineUiOrderingColorOnly( cmGroup );
    cmGroup->add( &m_overrideDataRange );
    if ( m_overrideDataRange() )
    {
        cmGroup->add( &m_userClipValue );
    }

    auto metaGroup = uiOrdering.addNewGroup( "File Information" );
    metaGroup->add( &m_metadata );
    metaGroup->setCollapsedByDefault();

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicData::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= "" */ )
{
    uiTreeOrdering.skipRemainingChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicData::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
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
    else if ( field == &m_userClipValue )
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
cvf::BoundingBox* RimSeismicData::boundingBox() const
{
    return m_boundingBox.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimSeismicData::zMin() const
{
    return std::abs( m_boundingBox->max().z() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimSeismicData::zMax() const
{
    return std::abs( m_boundingBox->min().z() );
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
int RimSeismicData::inlineMin() const
{
    return m_inlineInfo[0];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSeismicData::inlineMax() const
{
    return m_inlineInfo[1];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSeismicData::inlineStep() const
{
    return m_inlineInfo[2];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSeismicData::xlineMin() const
{
    return m_xlineInfo[0];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSeismicData::xlineMax() const
{
    return m_xlineInfo[1];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSeismicData::xlineStep() const
{
    return m_xlineInfo[2];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSeismicData::toInlineIndex( int inLine ) const
{
    int iIndex = inLine - inlineMin();
    iIndex /= inlineStep();

    return iIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSeismicData::toXlineIndex( int xLine ) const
{
    int xIndex = xLine - xlineMin();
    xIndex /= xlineStep();

    return xIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSeismicData::toZIndex( double z ) const
{
    int zIndex = (int)( ( z - zMin() ) / zStep() );
    return zIndex;
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
std::vector<double> RimSeismicData::alphaValues() const
{
    return m_clippedAlphaValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicData::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( ( changedField == &m_overrideDataRange ) || ( changedField == &m_userClipValue ) )
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
    m_clippedAlphaValues.clear();

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

    if ( updatePlot ) RiuMainWindow::instance()->seismicHistogramPanel()->showHistogram( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimSeismicData::convertToWorldCoords( int iLine, int xLine, double depth )
{
    if ( !openFileIfNotOpen() ) return { 0, 0, 0 };

    return m_filereader->convertToWorldCoords( iLine, xLine, depth );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<int, int> RimSeismicData::convertToInlineXline( cvf::Vec3d worldCoords )
{
    if ( !openFileIfNotOpen() ) return { 0, 0 };

    return m_filereader->convertToInlineXline( worldCoords[0], worldCoords[1] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RimSeismicData::dataRangeMinMax() const
{
    return m_activeDataRange;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<ZGYAccess::SeismicSliceData>
    RimSeismicData::sliceData( RiaDefines::SeismicSliceDirection direction, int sliceNumber, double zMin, double zMax )
{
    if ( !openFileIfNotOpen() ) return nullptr;

    int sliceIndex = 0;
    int zMinIndex  = toZIndex( zMin );
    int zMaxIndex  = toZIndex( zMax );

    switch ( direction )
    {
        case RiaDefines::SeismicSliceDirection::INLINE:
            sliceIndex = ( sliceNumber - m_inlineInfo[0] ) / m_inlineInfo[2];
            break;
        case RiaDefines::SeismicSliceDirection::XLINE:
            sliceIndex = ( sliceNumber - m_xlineInfo[0] ) / m_xlineInfo[2];
            break;
        case RiaDefines::SeismicSliceDirection::DEPTH:
            sliceIndex = (int)( 1.0 * ( sliceNumber - this->zMin() ) / m_zStep );
            break;
        default:
            return nullptr;
    }

    auto data = m_filereader->slice( direction, sliceIndex, zMinIndex, zMaxIndex - zMinIndex );

    double tmp                  = 0.0;
    float* pValue               = data->values();
    const auto [minVal, maxVal] = dataRangeMinMax();
    const int nSize             = data->size();

    for ( int i = 0; i < nSize; i++, pValue++ )
    {
        tmp = *pValue;
        if ( tmp < minVal )
            *pValue = minVal;
        else if ( tmp > maxVal )
            *pValue = maxVal;
    }

    return data;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig* RimSeismicData::legendConfig() const
{
    return m_legendConfig();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicAlphaMapper* RimSeismicData::alphaValueMapper() const
{
    return m_alphaValueMapper.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicData::initColorLegend()
{
    m_legendConfig->setColorLegend( RimRegularLegendConfig::mapToColorLegend( RimRegularLegendConfig::ColorRangesType::BLUE_WHITE_RED ) );
    m_legendConfig->setMappingMode( RimRegularLegendConfig::MappingType::LINEAR_CONTINUOUS );
    m_legendConfig->setRangeMode( RimLegendConfig::RangeModeType::USER_DEFINED );
    m_legendConfig->setCenterLegendAroundZero( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<ZGYAccess::SeismicSliceData>
    RimSeismicData::sliceData( double worldX1, double worldY1, double worldX2, double worldY2, double zMin, double zMax )
{
    if ( !openFileIfNotOpen() ) return nullptr;

    auto [startInline, startXline] = m_filereader->convertToInlineXline( worldX1, worldY1 );
    auto [stopInline, stopXline]   = m_filereader->convertToInlineXline( worldX2, worldY2 );

    int startInlineIndex = toInlineIndex( startInline );
    int startXlineIndex  = toXlineIndex( startXline );
    int stopInlineIndex  = toInlineIndex( stopInline );
    int stopXlineIndex   = toXlineIndex( stopXline );
    int zMinIndex        = toZIndex( zMin );
    int zMaxIndex        = toZIndex( zMax );

    int diffI = std::abs( startInlineIndex - stopInlineIndex );
    int diffX = std::abs( startXlineIndex - stopXlineIndex );

    int dirI = 1;
    if ( startInlineIndex > stopInlineIndex ) dirI = -1;
    int dirX = 1;
    if ( startXlineIndex > stopXlineIndex ) dirX = -1;

    const int zSize = zMaxIndex - zMinIndex;

    std::shared_ptr<ZGYAccess::SeismicSliceData> retdata;

    if ( diffI > diffX )
    {
        double dstepX = 1.0 * dirX * diffX / diffI;

        double xlined = 1.0 * startXlineIndex;

        retdata     = std::make_shared<ZGYAccess::SeismicSliceData>( diffI, zSize );
        float* pOut = retdata->values();

        for ( int iline = startInlineIndex; iline != stopInlineIndex; iline += dirI, xlined += dstepX )
        {
            int xline = (int)std::round( xlined );

            auto trace = m_filereader->trace( iline, xline, zMinIndex, zSize );

            if ( trace->size() != zSize )
            {
                memset( pOut, 0, zSize * sizeof( float ) );
            }
            else
            {
                memcpy( pOut, trace->values(), zSize * sizeof( float ) );
            }

            pOut += zSize;
        }
    }
    else
    {
        if ( diffX == 0 ) return nullptr;

        double dstepI = 1.0 * dirI * diffI / diffX;

        double ilined = 1.0 * startInlineIndex;

        retdata     = std::make_shared<ZGYAccess::SeismicSliceData>( diffX, zSize );
        float* pOut = retdata->values();

        for ( int xline = startXlineIndex; xline != stopXlineIndex; xline += dirX, ilined += dstepI )
        {
            int iline = (int)std::round( ilined );

            auto trace = m_filereader->trace( iline, xline, zMinIndex, zSize );

            if ( trace->size() != zSize )
            {
                memset( pOut, 0, zSize * sizeof( float ) );
            }
            else
            {
                memcpy( pOut, trace->values(), zSize * sizeof( float ) );
            }

            pOut += zSize;
        }
    }

    return retdata;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
float RimSeismicData::valueAt( cvf::Vec3d worldCoord )
{
    if ( openFileIfNotOpen() )
    {
        auto [iline, xline] = convertToInlineXline( worldCoord );

        int iIndex = toInlineIndex( iline );
        int xIndex = toXlineIndex( xline );
        int zIndex = toZIndex( std::abs( worldCoord[2] ) );

        auto slice = m_filereader->trace( iIndex, xIndex, zIndex, 1 );

        if ( slice->size() == 1 ) return slice->values()[0];
    }

    return std::nanf( "" );
}
