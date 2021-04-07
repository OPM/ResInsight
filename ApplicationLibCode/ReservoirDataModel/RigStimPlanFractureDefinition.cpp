/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 -     Statoil ASA
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

#include "RigStimPlanFractureDefinition.h"

#include "RiaEclipseUnitTools.h"
#include "RiaFractureDefines.h"
#include "RiaLogging.h"

#include "RigFractureCell.h"
#include "RigFractureGrid.h"
#include "RigStatisticsMath.h"

#include "RivWellFracturePartMgr.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
/// Internal functions
//--------------------------------------------------------------------------------------------------
size_t findMirrorXIndex( std::vector<double> xs );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const double RigStimPlanFractureDefinition::THRESHOLD_VALUE = 1e-5;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigStimPlanFractureDefinition::RigStimPlanFractureDefinition()
    : m_unitSet( RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN )
    , m_xMirrorMode( false )
    , m_topPerfTvd( HUGE_VAL )
    , m_bottomPerfTvd( HUGE_VAL )
    , m_topPerfMd( HUGE_VAL )
    , m_bottomPerfMd( HUGE_VAL )
    , m_formationDip( HUGE_VAL )
    , m_orientation( Orientation::UNDEFINED )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigStimPlanFractureDefinition::~RigStimPlanFractureDefinition()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::EclipseUnitSystem RigStimPlanFractureDefinition::unitSet() const
{
    return m_unitSet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigStimPlanFractureDefinition::xCount() const
{
    return m_Xs.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigStimPlanFractureDefinition::yCount() const
{
    return m_Ys.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigStimPlanFractureDefinition::minDepth() const
{
    return -minY();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigStimPlanFractureDefinition::maxDepth() const
{
    return -maxY();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigStimPlanFractureDefinition::topPerfTvd() const
{
    return m_topPerfTvd;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigStimPlanFractureDefinition::bottomPerfTvd() const
{
    return m_bottomPerfTvd;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigStimPlanFractureDefinition::minY() const
{
    return m_Ys[0];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigStimPlanFractureDefinition::maxY() const
{
    return m_Ys.back();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigStimPlanFractureDefinition::computeScaledXs( const std::vector<double>& xs, double scaleFactor )
{
    std::vector<double> scaledXs;

    // Scale using 0 as scaling anchor
    for ( double x : xs )
    {
        if ( scaleFactor != 1.0 ) x *= scaleFactor;
        scaledXs.push_back( x );
    }

    return scaledXs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigStimPlanFractureDefinition::computeScaledYs( const std::vector<double>& ys,
                                                                    double                     scaleFactor,
                                                                    double                     wellPathIntersectionY )
{
    std::vector<double> scaledYs;

    // Scale using wellPathIntersectionY as scaling anchor
    for ( double y : ys )
    {
        if ( scaleFactor != 1.0 ) y = ( y - wellPathIntersectionY ) * scaleFactor + wellPathIntersectionY;
        scaledYs.push_back( y );
    }

    return scaledYs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigStimPlanFractureDefinition::setTvdToTopPerf( double topPerfTvd )
{
    m_topPerfTvd = topPerfTvd;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigStimPlanFractureDefinition::setTvdToBottomPerf( double bottomPerfTvd )
{
    m_bottomPerfTvd = bottomPerfTvd;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigStimPlanFractureDefinition::topPerfMd() const
{
    return m_topPerfMd;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigStimPlanFractureDefinition::bottomPerfMd() const
{
    return m_bottomPerfMd;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigStimPlanFractureDefinition::setMdToTopPerf( double topPerfMd )
{
    m_topPerfMd = topPerfMd;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigStimPlanFractureDefinition::setMdToBottomPerf( double bottomPerfMd )
{
    m_bottomPerfMd = bottomPerfMd;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigStimPlanFractureDefinition::generateXsFromFileXs( bool xMirrorMode )
{
    m_xMirrorMode = xMirrorMode;
    m_Xs.clear();

    if ( m_xMirrorMode )
    {
        size_t            mirrorIndex = findMirrorXIndex( m_fileXs );
        std::list<double> xs;

        // Mirror positive X values
        xs.push_back( m_fileXs[mirrorIndex] );
        for ( size_t i = mirrorIndex + 1; i < m_fileXs.size(); i++ )
        {
            xs.push_front( -m_fileXs[i] );
            xs.push_back( m_fileXs[i] );
        }
        m_Xs = std::vector<double>( xs.begin(), xs.end() );
    }
    else
    {
        m_Xs = m_fileXs;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<double>>
    RigStimPlanFractureDefinition::generateDataLayoutFromFileDataLayout( std::vector<std::vector<double>> fileXYData ) const
{
    if ( m_xMirrorMode )
    {
        std::vector<std::vector<double>> xyData;
        size_t                           mirrorIndex = findMirrorXIndex( m_fileXs );

        for ( const auto& yData : fileXYData )
        {
            std::list<double> xValues;

            // Mirror positive X values
            xValues.push_back( yData[mirrorIndex] );
            for ( size_t x = mirrorIndex + 1; x < yData.size(); x++ )
            {
                xValues.push_front( yData[x] );
                xValues.push_back( yData[x] );
            }
            xyData.push_back( std::vector<double>( xValues.begin(), xValues.end() ) );
        }
        return xyData;
    }
    else
    {
        return fileXYData;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigStimPlanFractureDefinition::numberOfParameterValuesOK( std::vector<std::vector<double>> propertyValuesAtTimestep ) const
{
    size_t xCount = m_Xs.size();

    if ( propertyValuesAtTimestep.size() != yCount() ) return false;
    for ( const std::vector<double>& valuesAtDepthVector : propertyValuesAtTimestep )
    {
        if ( valuesAtDepthVector.size() != xCount ) return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double>
    RigStimPlanFractureDefinition::adjustedYCoordsAroundWellPathPosition( const std::vector<double>& ys,
                                                                          double wellPathIntersectionAtFractureDepth )
{
    std::vector<double> yRelativeToWellPath;

    for ( double y : ys )
    {
        double adjustedDepth = y + wellPathIntersectionAtFractureDepth;
        yRelativeToWellPath.push_back( adjustedDepth );
    }
    return yRelativeToWellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<QString, QString>> RigStimPlanFractureDefinition::getStimPlanPropertyNamesUnits() const
{
    std::vector<std::pair<QString, QString>> propertyNamesUnits;
    {
        for ( const RigStimPlanResultFrames& stimPlanDataEntry : this->m_stimPlanResults )
        {
            propertyNamesUnits.push_back( std::make_pair( stimPlanDataEntry.resultName, stimPlanDataEntry.unit ) );
        }
    }
    return propertyNamesUnits;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<double>>
    RigStimPlanFractureDefinition::conductivityValuesAtTimeStep( const QString&                resultName,
                                                                 int                           activeTimeStepIndex,
                                                                 RiaDefines::EclipseUnitSystem requiredUnitSet ) const
{
    std::vector<std::vector<double>> conductivityValues;

    QString conductivityUnitTextOnFile;

    std::vector<std::pair<QString, QString>> propertyNamesUnitsOnFile = this->getStimPlanPropertyNamesUnits();
    for ( auto properyNameUnit : propertyNamesUnitsOnFile )
    {
        if ( resultName == properyNameUnit.first )
        {
            conductivityUnitTextOnFile = properyNameUnit.second;
        }
    }

    if ( conductivityUnitTextOnFile.isEmpty() )
    {
        RiaLogging::error( "Did not find unit for conductivity on file" );

        return conductivityValues;
    }

    conductivityValues = this->getDataAtTimeIndex( resultName, conductivityUnitTextOnFile, activeTimeStepIndex );

    // Check that the data is in the required unit system.
    // Convert if not the case.
    if ( requiredUnitSet != unitSet() )
    {
        // Convert to the conductivity unit system used by the fracture template
        // The conductivity value is used in the computations of transmissibility when exporting COMPDAT, and has unit
        // md-m or md-ft This unit must match the unit used to represent coordinates of the grid used for export

        for ( auto& yValues : conductivityValues )
        {
            for ( auto& xVal : yValues )
            {
                if ( requiredUnitSet == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
                {
                    xVal = RiaEclipseUnitTools::convertToFeet( xVal, conductivityUnitTextOnFile );
                }
                else if ( requiredUnitSet == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
                {
                    xVal = RiaEclipseUnitTools::convertToMeter( xVal, conductivityUnitTextOnFile );
                }
            }
        }
    }

    return conductivityValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::cref<RigFractureGrid>
    RigStimPlanFractureDefinition::createFractureGrid( const QString& resultName,
                                                       int            activeTimeStepIndex,
                                                       double         xScaleFactor,
                                                       double         yScaleFactor,
                                                       double         wellPathIntersectionAtFractureDepth,
                                                       RiaDefines::EclipseUnitSystem requiredUnitSet ) const
{
    std::vector<std::vector<double>> conductivityValues =
        conductivityValuesAtTimeStep( resultName, activeTimeStepIndex, requiredUnitSet );
    if ( conductivityValues.empty() )
    {
        return nullptr;
    }

    std::vector<RigFractureCell> stimPlanCells;
    std::pair<size_t, size_t>    wellCenterStimPlanCellIJ = std::make_pair( 0, 0 );

    bool wellCenterStimPlanCellFound = false;

    std::vector<double> scaledXs = computeScaledXs( this->m_Xs, xScaleFactor );
    std::vector<double> scaledYs = computeScaledYs( this->m_Ys, yScaleFactor, -wellPathIntersectionAtFractureDepth );

    std::vector<double> yCoordsAtNodes =
        this->adjustedYCoordsAroundWellPathPosition( scaledYs, wellPathIntersectionAtFractureDepth );

    std::vector<double> xCoordsAtNodes = scaledXs;

    std::vector<double> xCoords;
    for ( int i = 0; i < static_cast<int>( xCoordsAtNodes.size() ) - 1; i++ )
        xCoords.push_back( ( xCoordsAtNodes[i] + xCoordsAtNodes[i + 1] ) / 2 );
    std::vector<double> depthCoords;
    for ( int i = 0; i < static_cast<int>( yCoordsAtNodes.size() ) - 1; i++ )
        depthCoords.push_back( ( yCoordsAtNodes[i] + yCoordsAtNodes[i + 1] ) / 2 );

    for ( int i = 0; i < static_cast<int>( xCoords.size() ) - 1; i++ )
    {
        for ( int j = 0; j < static_cast<int>( depthCoords.size() ) - 1; j++ )
        {
            std::vector<cvf::Vec3d> cellPolygon;
            cellPolygon.push_back( cvf::Vec3d( xCoords[i], depthCoords[j], 0.0 ) );
            cellPolygon.push_back( cvf::Vec3d( xCoords[i + 1], depthCoords[j], 0.0 ) );
            cellPolygon.push_back( cvf::Vec3d( xCoords[i + 1], depthCoords[j + 1], 0.0 ) );
            cellPolygon.push_back( cvf::Vec3d( xCoords[i], depthCoords[j + 1], 0.0 ) );

            RigFractureCell stimPlanCell( cellPolygon, i, j );
            if ( !conductivityValues.empty() ) // Assuming vector to be of correct length, or no values
            {
                stimPlanCell.setConductivityValue( conductivityValues[j + 1][i + 1] );
            }
            else
            {
                stimPlanCell.setConductivityValue( cvf::UNDEFINED_DOUBLE );
            }

            // The well path is intersecting the fracture at origo in the fracture coordinate system
            // Find the Stimplan cell where the well path is intersecting

            if ( cellPolygon[0].x() <= 0.0 && cellPolygon[1].x() >= 0.0 )
            {
                if ( cellPolygon[1].y() >= 0.0 && cellPolygon[2].y() <= 0.0 )
                {
                    wellCenterStimPlanCellIJ = std::make_pair( stimPlanCell.getI(), stimPlanCell.getJ() );
                    RiaLogging::debug(
                        QString( "Setting wellCenterStimPlanCell at cell %1, %2" )
                            .arg( QString::number( stimPlanCell.getI() ), QString::number( stimPlanCell.getJ() ) ) );

                    wellCenterStimPlanCellFound = true;
                }
            }

            stimPlanCells.push_back( stimPlanCell );
        }
    }

    if ( !wellCenterStimPlanCellFound )
    {
        RiaLogging::error( "Did not find stim plan cell at well crossing!" );
    }

    cvf::ref<RigFractureGrid> fractureGrid = new RigFractureGrid;
    fractureGrid->setFractureCells( stimPlanCells );
    fractureGrid->setWellCenterFractureCellIJ( wellCenterStimPlanCellIJ );
    fractureGrid->setICellCount( this->m_Xs.size() - 2 );
    fractureGrid->setJCellCount( this->m_Ys.size() - 2 );

    return cvf::cref<RigFractureGrid>( fractureGrid.p() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigStimPlanFractureDefinition::fractureGridResults( const QString& resultName,
                                                                        const QString& unitName,
                                                                        size_t         timeStepIndex ) const
{
    std::vector<double>                     fractureGridResults;
    const std::vector<std::vector<double>>& resultValuesAtTimeStep =
        this->getDataAtTimeIndex( resultName, unitName, timeStepIndex );

    for ( int i = 0; i < static_cast<int>( xCount() ) - 2; i++ )
    {
        for ( int j = 0; j < static_cast<int>( yCount() ) - 2; j++ )
        {
            if ( j + 1 < static_cast<int>( resultValuesAtTimeStep.size() ) &&
                 i + 1 < static_cast<int>( resultValuesAtTimeStep[j + 1].size() ) )
            {
                fractureGridResults.push_back( resultValuesAtTimeStep[j + 1][i + 1] );
            }
            else
            {
                fractureGridResults.push_back( HUGE_VAL );
            }
        }
    }

    return fractureGridResults;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigStimPlanFractureDefinition::createFractureTriangleGeometry( double         xScaleFactor,
                                                                    double         yScaleFactor,
                                                                    double         wellPathIntersectionAtFractureDepth,
                                                                    const QString& fractureUserName,
                                                                    std::vector<cvf::Vec3f>* vertices,
                                                                    std::vector<cvf::uint>*  triangleIndices ) const
{
    std::vector<double> scaledXs = computeScaledXs( this->m_Xs, xScaleFactor );
    std::vector<double> scaledYs = computeScaledYs( this->m_Ys, yScaleFactor, -wellPathIntersectionAtFractureDepth );

    std::vector<double> xCoords    = scaledXs;
    cvf::uint           lenXcoords = static_cast<cvf::uint>( xCoords.size() );

    std::vector<double> adjustedYs =
        this->adjustedYCoordsAroundWellPathPosition( scaledYs, wellPathIntersectionAtFractureDepth );

    for ( cvf::uint k = 0; k < adjustedYs.size(); k++ )
    {
        for ( cvf::uint i = 0; i < lenXcoords; i++ )
        {
            cvf::Vec3f node = cvf::Vec3f( xCoords[i], adjustedYs[k], 0 );
            vertices->push_back( node );

            if ( i < lenXcoords - 1 && k < adjustedYs.size() - 1 )
            {
                if ( xCoords[i] < THRESHOLD_VALUE )
                {
                    // Upper triangle
                    triangleIndices->push_back( i + k * lenXcoords );
                    triangleIndices->push_back( ( i + 1 ) + k * lenXcoords );
                    triangleIndices->push_back( ( i + 1 ) + ( k + 1 ) * lenXcoords );
                    // Lower triangle
                    triangleIndices->push_back( i + k * lenXcoords );
                    triangleIndices->push_back( ( i + 1 ) + ( k + 1 ) * lenXcoords );
                    triangleIndices->push_back( ( i ) + ( k + 1 ) * lenXcoords );
                }
                else
                {
                    // Upper triangle
                    triangleIndices->push_back( i + k * lenXcoords );
                    triangleIndices->push_back( ( i + 1 ) + k * lenXcoords );
                    triangleIndices->push_back( ( i ) + ( k + 1 ) * lenXcoords );
                    // Lower triangle
                    triangleIndices->push_back( ( i + 1 ) + k * lenXcoords );
                    triangleIndices->push_back( ( i + 1 ) + ( k + 1 ) * lenXcoords );
                    triangleIndices->push_back( ( i ) + ( k + 1 ) * lenXcoords );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigStimPlanFractureDefinition::timeSteps() const
{
    return m_timeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigStimPlanFractureDefinition::addTimeStep( double time )
{
    if ( !timeStepExists( time ) ) m_timeSteps.push_back( time );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigStimPlanFractureDefinition::timeStepExists( double timeStepValueToCheck ) const
{
    for ( double timeStep : m_timeSteps )
    {
        if ( fabs( timeStepValueToCheck - timeStep ) < THRESHOLD_VALUE ) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigStimPlanFractureDefinition::getTimeStepIndex( double timeStepValue ) const
{
    size_t index = 0;
    while ( index < m_timeSteps.size() )
    {
        if ( fabs( m_timeSteps[index] - timeStepValue ) < 1e-4 )
        {
            return index;
        }
        index++;
    }
    return -1; // returns -1 if not found
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigStimPlanFractureDefinition::totalNumberTimeSteps() const
{
    return m_timeSteps.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigStimPlanFractureDefinition::resultIndex( const QString& resultName, const QString& unit ) const
{
    for ( size_t i = 0; i < m_stimPlanResults.size(); i++ )
    {
        if ( m_stimPlanResults[i].resultName == resultName && m_stimPlanResults[i].unit == unit )
        {
            return i;
        }
    }

    return cvf::UNDEFINED_SIZE_T;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigStimPlanFractureDefinition::setDataAtTimeValue( const QString&                          resultName,
                                                        const QString&                          unit,
                                                        const std::vector<std::vector<double>>& data,
                                                        double                                  timeStepValue )
{
    size_t resIndex = resultIndex( resultName, unit );

    if ( resIndex != cvf::UNDEFINED_SIZE_T )
    {
        m_stimPlanResults[resIndex].parameterValues[getTimeStepIndex( timeStepValue )] = data;
    }
    else
    {
        RigStimPlanResultFrames resultData;

        resultData.resultName = resultName;
        resultData.unit       = unit;

        std::vector<std::vector<std::vector<double>>> values( m_timeSteps.size() );
        resultData.parameterValues                                    = values;
        resultData.parameterValues[getTimeStepIndex( timeStepValue )] = data;

        m_stimPlanResults.push_back( resultData );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<std::vector<double>>& RigStimPlanFractureDefinition::getDataAtTimeIndex( const QString& resultName,
                                                                                           const QString& unit,
                                                                                           size_t timeStepIndex ) const
{
    size_t resIndex = resultIndex( resultName, unit );

    if ( resIndex != cvf::UNDEFINED_SIZE_T )
    {
        if ( timeStepIndex < m_stimPlanResults[resIndex].parameterValues.size() )
        {
            return m_stimPlanResults[resIndex].parameterValues[timeStepIndex];
        }
    }

    RiaLogging::error( "Requested parameter does not exists in stimPlan data" );

    static std::vector<std::vector<double>> emptyVector;
    return emptyVector;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigStimPlanFractureDefinition::appendDataToResultStatistics( const QString&     resultName,
                                                                  const QString&     unit,
                                                                  MinMaxAccumulator& minMaxAccumulator,
                                                                  PosNegAccumulator& posNegAccumulator ) const
{
    size_t resIndex = resultIndex( resultName, unit );
    if ( resIndex == cvf::UNDEFINED_SIZE_T ) return;

    for ( const auto& timeValues : m_stimPlanResults[resIndex].parameterValues )
    {
        for ( const auto& values : timeValues )
        {
            minMaxAccumulator.addData( values );
            posNegAccumulator.addData( values );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RigStimPlanFractureDefinition::conductivityResultNames() const
{
    QStringList resultNames;

    for ( const auto& stimPlanResult : m_stimPlanResults )
    {
        if ( stimPlanResult.resultName.contains( RiaDefines::conductivityResultName(), Qt::CaseInsensitive ) )
        {
            resultNames.push_back( stimPlanResult.resultName );
        }
    }

    return resultNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t findMirrorXIndex( std::vector<double> xs )
{
    size_t mirrorIndex = cvf::UNDEFINED_SIZE_T;

    for ( size_t i = 0; i < xs.size(); i++ )
    {
        if ( xs[i] > -RigStimPlanFractureDefinition::THRESHOLD_VALUE )
        {
            mirrorIndex = i;
            break;
        }
    }

    return mirrorIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigStimPlanFractureDefinition::formationDip() const
{
    return m_formationDip;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigStimPlanFractureDefinition::setFormationDip( double formationDip )
{
    m_formationDip = formationDip;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigStimPlanFractureDefinition::Orientation RigStimPlanFractureDefinition::orientation() const
{
    return m_orientation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigStimPlanFractureDefinition::setOrientation( RigStimPlanFractureDefinition::Orientation orientation )
{
    m_orientation = orientation;
}
