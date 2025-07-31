
/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023  Equinor ASA
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

#include "RifOpenVDSReader.h"

#include <zgyaccess/seismicslice.h>
#include <zgyaccess/zgy_histogram.h>
#include <zgyaccess/zgyreader.h>

#if HAVE_OPENVDS
#include <OpenVDS/IJKCoordinateTransformer.h>
#include <OpenVDS/OpenVDS.h>
#include <OpenVDS/VolumeDataAccess.h>
#include <OpenVDS/VolumeDataLayout.h>
#endif // HAVE_OPENVDS

#include "cvfBoundingBox.h"

constexpr int VDS_INLINE_DIM = 2;
constexpr int VDS_XLINE_DIM  = 1;
constexpr int VDS_Z_DIM      = 0;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifOpenVDSReader::RifOpenVDSReader()
    : m_filename( "" )
    , m_dataChannelToUse( 0 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifOpenVDSReader::~RifOpenVDSReader()
{
    close();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpenVDSReader::open( QString filename )
{
    close();

    m_filename = filename;

#if HAVE_OPENVDS
    try
    {
        OpenVDS::Error error;
        m_handle = OpenVDS::Open( filename.toStdString(), "", error );

        if ( error.code != 0 )
        {
            m_handle = nullptr;
            return false;
        }

        auto accessManager = OpenVDS::GetAccessManager( m_handle );
        m_layout           = accessManager.GetVolumeDataLayout();
        if ( m_layout == nullptr )
        {
            close();
            return false;
        }
        m_coordinateTransform = std::make_unique<OpenVDS::IJKCoordinateTransformer>( m_layout );
    }
    catch ( const std::exception& )
    {
        m_handle = nullptr;
        return false;
    }

    return true;

#else // HAVE_OPENVDS
    m_handle = nullptr;
    return false;
#endif // HAVE_OPENVDS
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpenVDSReader::isOpen() const
{
    return ( m_handle != nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpenVDSReader::isValid()
{
    if ( !isOpen() ) return false;

#if HASE_OPENVDS
    auto iAxis = m_layout->GetAxisDescriptor( VDS_INLINE_DIM );
    auto xAxis = m_layout->GetAxisDescriptor( VDS_XLINE_DIM );
    auto zAxis = m_layout->GetAxisDescriptor( VDS_Z_DIM );

    return ( iAxis.GetCoordinateStep() > 0 ) && ( xAxis.GetCoordinateStep() > 0 ) && ( zAxis.GetCoordinateStep() > 0 );
#else // HAVE_OPENVDS
    return false;
#endif // HAVE_OPENVDS
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifOpenVDSReader::close()
{
    if ( !isOpen() ) return;

    #if HAVE_OPENVDS
    try
    {
        OpenVDS::Close( m_handle );
    }
    catch ( const std::exception& )
    {
    }

    m_handle = nullptr;
    m_layout = nullptr;
#endif // HAVE_OPENVDS

    return;
}

#if HAVE_OPENVDS
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString openvds_dataFormatToString( OpenVDS::VolumeDataFormat format )
{
    switch ( format )
    {
        case OpenVDS::VolumeDataFormat::Format_1Bit:
            return "1-bit";
        case OpenVDS::VolumeDataFormat::Format_U8:
            return "Unsigned 8-bit";
        case OpenVDS::VolumeDataFormat::Format_U16:
            return "Unsigned 16-bit";
        case OpenVDS::VolumeDataFormat::Format_R32:
            return "Float (32-bit)";
        case OpenVDS::VolumeDataFormat::Format_U32:
            return "Unsigned 32-bit";
        case OpenVDS::VolumeDataFormat::Format_R64:
            return "Double (64-bit)";
        case OpenVDS::VolumeDataFormat::Format_U64:
            return "Unsigned 64-bit";
        case OpenVDS::VolumeDataFormat::Format_Any:
        default:
            break;
    }

    return "Unknown";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString openvds_axisToString( OpenVDS::VolumeDataAxisDescriptor axis )
{
    return QString( "%1 - %2, step %3" ).arg( axis.GetCoordinateMin() ).arg( axis.GetCoordinateMax() ).arg( axis.GetCoordinateStep() );
}
#endif // HAVE_OPENVDS

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<QString, QString>> RifOpenVDSReader::metaData()
{
    std::vector<std::pair<QString, QString>> retValues;

    if ( !isOpen() ) return retValues;

#if HAVE_OPENVDS
    QString version( OpenVDS::GetOpenVDSVersion() );
    retValues.push_back( std::make_pair( QString( "Open VDS version" ), QString( OpenVDS::GetOpenVDSVersion() ) ) );

    QString compressed( "No" );
    if ( OpenVDS::GetCompressionMethod( m_handle ) != OpenVDS::CompressionMethod::None ) compressed = "Yes";
    retValues.push_back( std::make_pair( QString( "Compression" ), compressed ) );

    retValues.push_back( std::make_pair( QString( "Inline" ), openvds_axisToString( m_layout->GetAxisDescriptor( VDS_INLINE_DIM ) ) ) );
    retValues.push_back( std::make_pair( QString( "Xline" ), openvds_axisToString( m_layout->GetAxisDescriptor( VDS_XLINE_DIM ) ) ) );
    retValues.push_back( std::make_pair( QString( "Z" ), openvds_axisToString( m_layout->GetAxisDescriptor( VDS_Z_DIM ) ) ) );

    const int dimensions = m_layout->GetDimensionality();
    retValues.push_back( std::make_pair( QString( "Dimensions" ), QString::number( dimensions ) ) );

    const int channels = m_layout->GetChannelCount();
    retValues.push_back( std::make_pair( QString( "Data Channels" ), QString::number( channels ) ) );

    for ( int i = 0; i < channels; i++ )
    {
        QString prefix = QString( "Data Channel %1 " ).arg( i );

        auto chanDesc = m_layout->GetChannelDescriptor( i );

        QString chanName( chanDesc.GetName() );
        retValues.push_back( std::make_pair( prefix + "Name", chanName ) );
        retValues.push_back( std::make_pair( prefix + "Format", openvds_dataFormatToString( chanDesc.GetFormat() ) ) );
        QString range = QString( "%1 - %2" ).arg( chanDesc.GetValueRangeMin() ).arg( chanDesc.GetValueRangeMax() );
        retValues.push_back( std::make_pair( prefix + "Range", range ) );

        if ( chanName.toLower() == "amplitude" ) m_dataChannelToUse = i;
    }
#endif // HAVE_OPENVDS

    return retValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifOpenVDSReader::histogramData( std::vector<double>& xvals, std::vector<double>& yvals )
{
    if ( !isOpen() ) return;

#if HAVE_OPENVDS
    auto iMinMaxStep = inlineMinMaxStep();
    auto xMinMaxStep = xlineMinMaxStep();

    int voxelMin[OpenVDS::Dimensionality_Max] = { 0, 0, 0, 0, 0, 0 };
    int voxelMax[OpenVDS::Dimensionality_Max] = { 1, 1, 1, 1, 1, 1 };

    const int zMax = zSize();

    const int iSize = ( iMinMaxStep[1] - iMinMaxStep[0] ) / iMinMaxStep[2];
    const int xSize = ( xMinMaxStep[1] - xMinMaxStep[0] ) / xMinMaxStep[2];

    voxelMin[VDS_Z_DIM] = zMax / 4;
    voxelMax[VDS_Z_DIM] = zMax - zMax / 4;

    voxelMin[VDS_XLINE_DIM] = xSize / 4;
    voxelMax[VDS_XLINE_DIM] = xSize - xSize / 4;

    voxelMin[VDS_INLINE_DIM] = iSize / 4;
    voxelMax[VDS_INLINE_DIM] = iSize - iSize / 4;

    const int totalSize = ( voxelMax[VDS_Z_DIM] - voxelMin[VDS_Z_DIM] ) * ( voxelMax[VDS_XLINE_DIM] - voxelMin[VDS_XLINE_DIM] ) *
                          ( voxelMax[VDS_INLINE_DIM] - voxelMin[VDS_INLINE_DIM] );

    std::vector<float> buffer( totalSize );

    auto accessManager = OpenVDS::GetAccessManager( m_handle );

    bool success = false;

    try
    {
        auto request = accessManager.RequestVolumeSubset<float>( buffer.data(),
                                                                 buffer.size() * sizeof( float ),
                                                                 OpenVDS::Dimensions_012,
                                                                 0,
                                                                 m_dataChannelToUse,
                                                                 voxelMin,
                                                                 voxelMax );

        success = request->WaitForCompletion();
    }
    catch ( const std::exception& )
    {
    }

    if ( success )
    {
        auto chanDesc = m_layout->GetChannelDescriptor( m_dataChannelToUse );

        m_histogram =
            ZGYAccess::HistogramGenerator::getHistogram( buffer, 151, (float)chanDesc.GetValueRangeMin(), (float)chanDesc.GetValueRangeMax() );

        xvals = m_histogram->Xvalues;
        yvals = m_histogram->Yvalues;
    }
#endif // HAVE_OPENVDS
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RifOpenVDSReader::dataRange()
{
    if ( !isOpen() ) return { 0.0, 0.0 };
#if HAVE_OPENVDS
    auto chanDesc = m_layout->GetChannelDescriptor( m_dataChannelToUse );
    return { chanDesc.GetValueRangeMin(), chanDesc.GetValueRangeMax() };
#else // HAVE_OPENVDS
    return { 0.0, 0.0 };
#endif // HAVE_OPENVDS
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RifOpenVDSReader::worldCorners()
{
    if ( !isOpen() ) return {};

#if HAVE_OPENVDS
    auto iAxis = m_layout->GetAxisDescriptor( VDS_INLINE_DIM );
    auto xAxis = m_layout->GetAxisDescriptor( VDS_XLINE_DIM );
    auto zAxis = m_layout->GetAxisDescriptor( VDS_Z_DIM );

    const float iMin = iAxis.GetCoordinateMin();
    const float iMax = iAxis.GetCoordinateMax();
    const float xMin = xAxis.GetCoordinateMin();
    const float xMax = xAxis.GetCoordinateMax();
    const float zMin = zAxis.GetCoordinateMin();
    const float zMax = zAxis.GetCoordinateMax();

    cvf::Vec3dArray annotPoints;
    annotPoints.resize( 8 );
    annotPoints[0] = cvf::Vec3d( iMin, xMin, zMin );
    annotPoints[1] = cvf::Vec3d( iMax, xMin, zMin );
    annotPoints[2] = cvf::Vec3d( iMin, xMax, zMin );
    annotPoints[3] = cvf::Vec3d( iMax, xMax, zMin );
    annotPoints[4] = cvf::Vec3d( iMin, xMin, zMax );
    annotPoints[5] = cvf::Vec3d( iMax, xMin, zMax );
    annotPoints[6] = cvf::Vec3d( iMin, xMax, zMax );
    annotPoints[7] = cvf::Vec3d( iMax, xMax, zMax );

    std::vector<cvf::Vec3d> retval;

    for ( auto p : annotPoints )
    {
        auto world = m_coordinateTransform->AnnotationToWorld( OpenVDS::DoubleVector3( p.x(), p.y(), p.z() ) );

        retval.push_back( cvf::Vec3d( world.X, world.Y, world.Z ) );
    }

    return retval;
#else // HAVE_OPENVDS
    return {};
#endif // HAVE_OPENVDS
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RifOpenVDSReader::zStep()
{
    if ( !isOpen() ) return 0.0;
#if HAVE_OPENVDS
    return m_layout->GetAxisDescriptor( VDS_Z_DIM ).GetCoordinateStep();
#else // HAVE_OPENVDS
    return 0.0;
#endif // HAVE_OPENVDS
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifOpenVDSReader::zSize()
{
    if ( !isOpen() ) return 0;
#if HAVE_OPENVDS
    return m_layout->GetAxisDescriptor( VDS_Z_DIM ).GetNumSamples();
#else // HAVE_OPENVDS
    return 0;
#endif // HAVE_OPENVDS
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3i RifOpenVDSReader::minMaxStep( int dimension )
{
    if ( !isOpen() ) return { 0, 0, 0 };

#if HAVE_OPENVDS
    auto axis = m_layout->GetAxisDescriptor( dimension );

    return { (int)( axis.GetCoordinateMin() + 0.5 ), (int)( axis.GetCoordinateMax() + 0.5 ), (int)( axis.GetCoordinateStep() + 0.5 ) };
#else // HAVE_OPENVDS
    return { 0, 0, 0 };
#endif // HAVE_OPENVDS
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3i RifOpenVDSReader::inlineMinMaxStep()
{
#if HAVE_OPENVDS
    return minMaxStep( VDS_INLINE_DIM );
#else // HAVE_OPENVDS
    return { 0, 0, 0 };
#endif // HAVE_OPENVDS
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3i RifOpenVDSReader::xlineMinMaxStep()
{
#if HAVE_OPENVDS
    return minMaxStep( VDS_XLINE_DIM );
#else // HAVE_OPENVDS
    return { 0, 0, 0 };
#endif // HAVE_OPENVDS
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RifOpenVDSReader::convertToWorldCoords( int iLine, int xLine, double depth )
{
    if ( !isOpen() ) return { 0, 0, 0 };

#if HAVE_OPENVDS
    auto world = m_coordinateTransform->AnnotationToWorld( OpenVDS::DoubleVector3( iLine, xLine, depth ) );
    return { world.X, world.Y, depth };
#else // HAVE_OPENVDS
    return { 0, 0, 0 };
#endif // HAVE_OPENVDS
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<int, int> RifOpenVDSReader::convertToInlineXline( double worldx, double worldy )
{
    if ( !isOpen() ) return { 0, 0 };

#if HAVE_OPENVDS
    auto annot = m_coordinateTransform->WorldToAnnotation( OpenVDS::DoubleVector3( worldx, worldy, 0 ) );
    return { (int)( annot.X + 0.5 ), (int)( annot.Y + 0.5 ) };
#else // HAVE_OPENVDS
    return { 0, 0 };
#endif // HAVE_OPENVDS
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<ZGYAccess::SeismicSliceData>
    RifOpenVDSReader::slice( RiaDefines::SeismicSliceDirection direction, int sliceIndex, int zStartIndex, int zSize )
{
    if ( !isOpen() ) return nullptr;

#if HAVE_OPENVDS
    if ( zStartIndex < 0 )
    {
        zStartIndex = 0;
        zSize       = this->zSize();
    }

    const int xlineSize  = m_layout->GetAxisDescriptor( VDS_XLINE_DIM ).GetNumSamples();
    const int inlineSize = m_layout->GetAxisDescriptor( VDS_INLINE_DIM ).GetNumSamples();

    int voxelMin[OpenVDS::Dimensionality_Max] = { 0, 0, 0, 0, 0, 0 };
    int voxelMax[OpenVDS::Dimensionality_Max] = { 1, 1, 1, 1, 1, 1 };

    int width  = 0;
    int height = 0;

    switch ( direction )
    {
        case RiaDefines::SeismicSliceDirection::INLINE:
            voxelMin[VDS_Z_DIM]      = zStartIndex;
            voxelMax[VDS_Z_DIM]      = zStartIndex + zSize;
            voxelMin[VDS_XLINE_DIM]  = 0;
            voxelMax[VDS_XLINE_DIM]  = xlineSize;
            voxelMin[VDS_INLINE_DIM] = sliceIndex;
            voxelMax[VDS_INLINE_DIM] = sliceIndex + 1;
            width                    = xlineSize;
            height                   = zSize;
            break;

        case RiaDefines::SeismicSliceDirection::XLINE:
            voxelMin[VDS_Z_DIM]      = zStartIndex;
            voxelMax[VDS_Z_DIM]      = zStartIndex + zSize;
            voxelMin[VDS_XLINE_DIM]  = sliceIndex;
            voxelMax[VDS_XLINE_DIM]  = sliceIndex + 1;
            voxelMin[VDS_INLINE_DIM] = 0;
            voxelMax[VDS_INLINE_DIM] = inlineSize;
            width                    = inlineSize;
            height                   = zSize;
            break;

        case RiaDefines::SeismicSliceDirection::DEPTH:
            voxelMin[VDS_Z_DIM]      = sliceIndex;
            voxelMax[VDS_Z_DIM]      = sliceIndex + 1;
            voxelMin[VDS_XLINE_DIM]  = 0;
            voxelMax[VDS_XLINE_DIM]  = xlineSize;
            voxelMin[VDS_INLINE_DIM] = 0;
            voxelMax[VDS_INLINE_DIM] = inlineSize;
            width                    = inlineSize;
            height                   = xlineSize;
            break;

        default:
            return nullptr;
    }

    int totalSize = width * height;

    std::shared_ptr<ZGYAccess::SeismicSliceData> retData = std::make_shared<ZGYAccess::SeismicSliceData>( width, height );

    auto accessManager = OpenVDS::GetAccessManager( m_handle );

    bool success = false;

    try
    {
        auto request = accessManager.RequestVolumeSubset<float>( retData->values(),
                                                                 totalSize * sizeof( float ),
                                                                 OpenVDS::Dimensions_012,
                                                                 0,
                                                                 m_dataChannelToUse,
                                                                 voxelMin,
                                                                 voxelMax );

        success = request->WaitForCompletion();
    }
    catch ( const std::exception& )
    {
    }

    if ( !success ) retData.reset();

    return retData;
#else // HAVE_OPENVDS
    return nullptr;
#endif // HAVE_OPENVDS
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<ZGYAccess::SeismicSliceData> RifOpenVDSReader::trace( int inlineIndex, int xlineIndex, int zStartIndex, int zSize )
{
    if ( !isOpen() ) return nullptr;

#if HAVE_OPENVDS
    if ( zStartIndex < 0 )
    {
        zStartIndex = 0;
        zSize       = this->zSize();
    }

    int voxelMin[OpenVDS::Dimensionality_Max] = { 0, 0, 0, 0, 0, 0 };
    int voxelMax[OpenVDS::Dimensionality_Max] = { 1, 1, 1, 1, 1, 1 };

    voxelMin[VDS_Z_DIM] = zStartIndex;
    voxelMax[VDS_Z_DIM] = zStartIndex + zSize;

    voxelMin[VDS_XLINE_DIM] = xlineIndex;
    voxelMax[VDS_XLINE_DIM] = xlineIndex + 1;

    voxelMin[VDS_INLINE_DIM] = inlineIndex;
    voxelMax[VDS_INLINE_DIM] = inlineIndex + 1;

    std::shared_ptr<ZGYAccess::SeismicSliceData> retData = std::make_shared<ZGYAccess::SeismicSliceData>( 1, zSize );

    auto accessManager = OpenVDS::GetAccessManager( m_handle );

    bool success = false;

    try
    {
        auto request = accessManager.RequestVolumeSubset<float>( retData->values(),
                                                                 zSize * sizeof( float ),
                                                                 OpenVDS::Dimensions_012,
                                                                 0,
                                                                 m_dataChannelToUse,
                                                                 voxelMin,
                                                                 voxelMax );

        success = request->WaitForCompletion();
    }
    catch ( const std::exception& )
    {
    }

    if ( !success ) retData.reset();

    return retData;
#else // HAVE_OPENVDS
    return nullptr;
#endif // HAVE_OPENVDS
}
