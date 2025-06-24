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

#include "Rim3dView.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimRegularLegendConfig.h"
#include "RimSeismicAlphaMapper.h"
#include "RimSeismicData.h"
#include "RimSeismicDataCollection.h"
#include "RimStringParameter.h"
#include "RimTools.h"

#include "RiuMainWindow.h"
#include "RiuSeismicHistogramPanel.h"

#include <zgyaccess/seismicslice.h>
#include <zgyaccess/zgy_histogram.h>

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
    , m_inputDataOK( false )
{
    CAF_PDM_InitObject( "SeismicDifferenceData", ":/SeismicDelta16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_userDescription, "SeismicUserDecription", "Name" );

    CAF_PDM_InitFieldNoDefault( &m_nameProxy, "NameProxy", "Name Proxy" );
    m_nameProxy.registerGetMethod( this, &RimSeismicDifferenceData::fullName );
    m_nameProxy.uiCapability()->setUiReadOnly( true );
    m_nameProxy.uiCapability()->setUiHidden( true );
    m_nameProxy.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_seismicData1, "SeismicData1", "Seismic Data 1" );
    CAF_PDM_InitFieldNoDefault( &m_seismicData2, "SeismicData2", "Seismic Data 2" );

    setDeletable( true );
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
    return m_inputDataOK && ( m_seismicData1 != nullptr ) && ( m_seismicData2 != nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimSeismicDifferenceData::userDescription() const
{
    return fullName().toStdString();
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
    return &m_nameProxy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSeismicDifferenceData::fullName() const
{
    QString name = m_userDescription();

    if ( name.isEmpty() ) name = "Difference";

    if ( isInputDataOK() )
    {
        name += QString( " [%1 - %2]" )
                    .arg( QString::fromStdString( m_seismicData1->userDescription() ) )
                    .arg( QString::fromStdString( m_seismicData2->userDescription() ) );
    }
    else
    {
        name += "  [Input data not valid]";
    }

    return name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSeismicDifferenceData::hasValidData() const
{
    return m_inputDataOK && ( m_seismicData1 != nullptr ) && ( m_seismicData2 != nullptr );
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

    if ( auto seis1 = dynamic_cast<RimSeismicData*>( m_seismicData1() ) )
    {
        seis1->ensureFileReaderIsInitialized();
    }
    if ( auto seis2 = dynamic_cast<RimSeismicData*>( m_seismicData2() ) )
    {
        seis2->ensureFileReaderIsInitialized();
    }

    m_inputDataOK = m_seismicData1->gridIsEqual( m_seismicData2 );

    if ( !m_inputDataOK ) return;

    m_boundingBox->reset();
    m_boundingBox->add( *m_seismicData1->boundingBox() );

    auto [min1, max1] = m_seismicData1->dataRangeMinMax();
    auto [min2, max2] = m_seismicData2->dataRangeMinMax();

    m_fileDataRange = std::make_pair( std::min( min1, min2 ), std::max( max1, max2 ) );

    generateHistogram();

    auto [minDataValue, maxDataValue] = m_fileDataRange;
    double maxAbsDataValue            = std::max( std::abs( minDataValue ), std::abs( maxDataValue ) );

    auto [userClipEnabled, userClipValue] = m_userClipValue();
    if ( userClipValue <= 0.0 ) userClipValue = maxAbsDataValue;
    userClipValue   = std::clamp( userClipValue, 0.0, maxAbsDataValue );
    m_userClipValue = std::make_pair( userClipEnabled, userClipValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicDifferenceData::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto genGroup = uiOrdering.addNewGroup( "General" );
    genGroup->add( &m_userDescription );

    auto diffGroup = uiOrdering.addNewGroup( "Input Data" );
    diffGroup->add( &m_seismicData1 );
    diffGroup->add( &m_seismicData2 );

    auto cmGroup = uiOrdering.addNewGroup( "Color Mapping" );
    m_legendConfig->defineUiOrderingColorOnly( cmGroup );
    cmGroup->add( &m_userClipValue );

    if ( !m_userClipValue().first )
    {
        cmGroup->add( &m_userMinMaxEnabled );
        if ( m_userMinMaxEnabled )
        {
            cmGroup->add( &m_userMinValue );
            cmGroup->add( &m_userMaxValue );
            m_userMuteThreshold = std::make_pair( false, m_userMuteThreshold().second );
        }
        else
        {
            cmGroup->add( &m_userMuteThreshold );
        }
    }
    else
    {
        cmGroup->add( &m_userMuteThreshold );
        m_userMinMaxEnabled = false;
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
void RimSeismicDifferenceData::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( ( changedField == &m_userMuteThreshold ) || ( changedField == &m_userClipValue ) || ( changedField == &m_userMinMaxEnabled ) ||
         ( changedField == &m_userMaxValue ) || ( changedField == &m_userMinValue ) )
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
std::pair<double, double> RimSeismicDifferenceData::sourceDataRangeMinMax() const
{
    if ( !isInputDataOK() ) return { 0, 0 };
    return m_fileDataRange;
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

    if ( ( data1 == nullptr ) || ( data2 == nullptr ) ) return nullptr;

    auto retdata = difference( data1.get(), data2.get() );

    const auto [minVal, maxVal] = dataRangeMinMax();
    retdata->limitTo( minVal, maxVal );

    auto [doMute, muteThreshold] = m_userMuteThreshold();
    if ( doMute ) retdata->mute( muteThreshold );

    return retdata;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<ZGYAccess::SeismicSliceData>
    RimSeismicDifferenceData::sliceData( double worldX1, double worldY1, double worldX2, double worldY2, double zMin, double zMax )
{
    if ( !isInputDataOK() ) return nullptr;

    auto data1 = m_seismicData1->sliceData( worldX1, worldY1, worldX2, worldY2, zMin, zMax );
    auto data2 = m_seismicData2->sliceData( worldX1, worldY1, worldX2, worldY2, zMin, zMax );

    if ( ( data1 == nullptr ) || ( data2 == nullptr ) ) return nullptr;

    auto retdata = difference( data1.get(), data2.get() );

    const auto [minVal, maxVal] = dataRangeMinMax();
    retdata->limitTo( minVal, maxVal );

    auto [doMute, muteThreshold] = m_userMuteThreshold();
    if ( doMute ) retdata->mute( muteThreshold );

    return retdata;
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSeismicDifferenceData::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_seismicData1 )
    {
        RimTools::seismicDataOptionItems( &options );
    }
    else if ( fieldNeedingOptions == &m_seismicData2 )
    {
        RimProject* proj = RimProject::current();
        if ( ( proj != nullptr ) && ( m_seismicData1() != nullptr ) )
        {
            const auto& coll = proj->activeOilField()->seismicDataCollection().p();

            for ( auto seisData : coll->seismicData() )
            {
                if ( m_seismicData1()->gridIsEqual( seisData ) )
                {
                    options.push_back( caf::PdmOptionItemInfo( QString::fromStdString( seisData->userDescription() ),
                                                               seisData,
                                                               false,
                                                               seisData->uiIconProvider() ) );
                }
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<ZGYAccess::SeismicSliceData> RimSeismicDifferenceData::difference( ZGYAccess::SeismicSliceData* data1,
                                                                                   ZGYAccess::SeismicSliceData* data2 )
{
    std::shared_ptr<ZGYAccess::SeismicSliceData> retData = std::make_shared<ZGYAccess::SeismicSliceData>( data1->width(), data1->depth() );

    float* pInput1 = data1->values();
    float* pInput2 = data2->values();
    float* pOutput = retData->values();

    const int nValues = retData->size();

    for ( int i = 0; i < nValues; i++, pInput1++, pInput2++, pOutput++ )
    {
        *pOutput = *pInput1 - *pInput2;
    }

    return retData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicDifferenceData::generateHistogram()
{
    auto [minVal, maxVal] = m_fileDataRange;

    auto generator = std::make_unique<ZGYAccess::HistogramGenerator>( 200, minVal, maxVal );

    const int iLineMin  = m_seismicData1->inlineMin();
    const int iLineMax  = m_seismicData1->inlineMax();
    const int iLineStep = m_seismicData1->inlineStep();

    const double zMin = m_seismicData1->zMin();
    const double zMax = m_seismicData1->zMax();

    for ( int iline = iLineMin; iline <= iLineMax; iline += iLineStep )
    {
        auto slicedata = sliceData( RiaDefines::SeismicSliceDirection::INLINE, iline, zMin, zMax );

        if ( slicedata->size() > 0 )
        {
            std::vector<float> vec( slicedata->values(), slicedata->values() + slicedata->size() );

            generator->addData( vec );

            minVal = std::min( minVal, (double)*std::min_element( vec.begin(), vec.end() ) );
            maxVal = std::max( maxVal, (double)*std::max_element( vec.begin(), vec.end() ) );
        }
    }

    auto hist = generator->getHistogram();

    m_histogramXvalues = hist->Xvalues;
    m_histogramYvalues = hist->Yvalues;

    m_fileDataRange = std::make_pair( minVal, maxVal );
}
