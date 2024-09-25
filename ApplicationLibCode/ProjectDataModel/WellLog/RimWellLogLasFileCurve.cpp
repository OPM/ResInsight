/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RimWellLogLasFileCurve.h"

#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "RiaQDateTimeTools.h"
#include "RiaResultNames.h"
#include "RigWellLogCurveData.h"
#include "RigWellLogIndexDepthOffset.h"
#include "RigWellPath.h"

#include "RimProject.h"
#include "RimTools.h"
#include "RimWellLogChannel.h"
#include "RimWellLogLasFile.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPlotTools.h"
#include "RimWellRftPlot.h"

#include "RiuQwtPlotCurve.h"
#include "RiuQwtPlotWidget.h"

#include "cafPdmUiTreeOrdering.h"

#include <QFileInfo>

CAF_PDM_SOURCE_INIT( RimWellLogLasFileCurve, "WellLogLasFileCurve", "WellLogFileCurve" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogLasFileCurve::RimWellLogLasFileCurve()
{
    CAF_PDM_InitObject( "Well Log File Curve", RimWellLogCurve::wellLogCurveIconName() );

    CAF_PDM_InitFieldNoDefault( &m_wellPath, "CurveWellPath", "Well Path" );
    m_wellPath.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_wellLogChannelName, "CurveWellLogChannel", "Well Log Channel" );

    CAF_PDM_InitFieldNoDefault( &m_wellLog, "WellLog", "Well Log" );
    m_wellLog.registerKeywordAlias( "WellLogFile" );

    m_wellPath = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogLasFileCurve::~RimWellLogLasFileCurve()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogLasFileCurve::onLoadDataAndUpdate( bool updateParentPlot )
{
    RimPlotCurve::updateCurvePresentation( updateParentPlot );

    if ( isChecked() )
    {
        auto wellLogPlot = firstAncestorOrThisOfTypeAsserted<RimWellLogPlot>();

        if ( m_wellPath && m_wellLog )
        {
            RigWellLogData* wellLogFile = m_wellLog->wellLogData();
            if ( wellLogFile )
            {
                std::vector<double> values              = wellLogFile->values( m_wellLogChannelName );
                std::vector<double> measuredDepthValues = wellLogFile->depthValues();
                std::vector<double> tvdMslValues        = wellLogFile->tvdMslValues();
                std::vector<double> tvdRkbValues        = wellLogFile->tvdRkbValues();

                bool rkbDiff = m_wellPath->wellPathGeometry() ? m_wellPath->wellPathGeometry()->rkbDiff() : 0.0;

                if ( m_indexDepthOffsets )
                {
                    // Adjust depths by reassigning depths for top and bottom of layer for each K layer
                    std::vector<double> kIndexValues = wellLogFile->values( RiaResultNames::indexKResultName() );
                    if ( values.size() == kIndexValues.size() )
                    {
                        auto [valuesAdjusted, measuredDepthValuesAdjusted] =
                            adjustByIndexDepthOffsets( measuredDepthValues, values, kIndexValues );

                        values              = valuesAdjusted;
                        measuredDepthValues = measuredDepthValuesAdjusted;
                    }
                }

                if ( tvdMslValues.size() != values.size() )
                {
                    RigWellPath* rigWellPath = m_wellPath->wellPathGeometry();
                    if ( rigWellPath )
                    {
                        tvdMslValues.clear();
                        for ( double measuredDepthValue : measuredDepthValues )
                        {
                            tvdMslValues.push_back( -rigWellPath->interpolatedPointAlongWellPath( measuredDepthValue ).z() );
                        }
                    }
                }
                if ( tvdRkbValues.size() != values.size() )
                {
                    RigWellPath* rigWellPath = m_wellPath->wellPathGeometry();
                    if ( rigWellPath )
                    {
                        tvdRkbValues.clear();
                        for ( double measuredDepthValue : measuredDepthValues )
                        {
                            tvdRkbValues.push_back( -rigWellPath->interpolatedPointAlongWellPath( measuredDepthValue ).z() +
                                                    m_wellPath->wellPathGeometry()->rkbDiff() );
                        }
                    }
                }

                std::map<RiaDefines::DepthTypeEnum, std::vector<double>> validDepths;
                if ( values.size() == measuredDepthValues.size() )
                {
                    validDepths.insert( std::make_pair( RiaDefines::DepthTypeEnum::MEASURED_DEPTH, measuredDepthValues ) );
                }
                if ( values.size() == tvdMslValues.size() )
                {
                    validDepths.insert( std::make_pair( RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH, tvdMslValues ) );
                }
                if ( values.size() == tvdRkbValues.size() )
                {
                    validDepths.insert( std::make_pair( RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH_RKB, tvdRkbValues ) );
                }

                bool useLogarithmicScale = false;
                auto track               = firstAncestorOfType<RimWellLogTrack>();
                if ( track )
                {
                    useLogarithmicScale = track->isLogarithmicScale();
                }

                setPropertyValuesAndDepths( values, validDepths, rkbDiff, wellLogFile->depthUnit(), false, useLogarithmicScale );

                QString errMsg;
                if ( wellLogPlot && !curveData()->availableDepthTypes().count( wellLogPlot->depthType() ) )
                {
                    QString depthTitle = wellLogPlot->depthAxisTitle();
                    errMsg             = QString( "Display of %1 for LAS curves is not possible without %2 "
                                                  "values in the LAS-file or a well path to derive them from." )
                                 .arg( depthTitle )
                                 .arg( depthTitle );
                }

                if ( !errMsg.isEmpty() )
                {
                    QString tmp = QString( "The LAS curve can not be displayed.\n%1\n" ).arg( errMsg );

                    RiaLogging::warning( tmp );
                }
            }

            if ( m_namingMethod == RiaDefines::ObjectNamingMethod::AUTO )
            {
                m_plotCurve->setTitle( createCurveAutoName() );
            }
        }

        RiaDefines::DepthUnitType displayUnit = RiaDefines::DepthUnitType::UNIT_METER;
        if ( wellLogPlot )
        {
            displayUnit = wellLogPlot->depthUnit();
        }

        RiaDefines::DepthTypeEnum depthType = RiaDefines::DepthTypeEnum::MEASURED_DEPTH;
        if ( wellLogPlot && curveData()->availableDepthTypes().count( wellLogPlot->depthType() ) )
        {
            depthType = wellLogPlot->depthType();
        }

        setPropertyAndDepthValuesToPlotCurve( curveData()->propertyValuesByIntervals(),
                                              curveData()->depthValuesByIntervals( depthType, displayUnit ) );

        if ( updateParentPlot )
        {
            updateZoomInParentPlot();
        }

        if ( m_parentPlot )
        {
            m_parentPlot->replot();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<double>, std::vector<double>>
    RimWellLogLasFileCurve::adjustByIndexDepthOffsets( const std::vector<double>& measuredDepthValues,
                                                       const std::vector<double>& values,
                                                       const std::vector<double>& kIndexValues ) const
{
    CAF_ASSERT( values.size() == kIndexValues.size() );

    auto findFirstIndex = []( int kLayer, const std::vector<double>& vals )
    {
        for ( size_t i = 0; i < vals.size(); i++ )
            if ( kLayer == static_cast<int>( vals[i] ) ) return i;

        return vals.size();
    };

    auto findLastIndex = []( int kLayer, const std::vector<double>& vals )
    {
        for ( int i = static_cast<int>( vals.size() ) - 1; i >= 0; i-- )
            if ( kLayer == static_cast<int>( vals[i] ) ) return static_cast<size_t>( i );

        return vals.size();
    };

    std::vector<int> kIndexes = m_indexDepthOffsets->sortedIndexes();

    std::vector<double> valuesAdjusted;
    std::vector<double> measuredDepthValuesAdjusted;

    for ( int kLayer : kIndexes )
    {
        size_t firstIndex = findFirstIndex( kLayer, kIndexValues );
        size_t lastIndex  = findLastIndex( kLayer, kIndexValues );

        if ( firstIndex != kIndexValues.size() && lastIndex != kIndexValues.size() )
        {
            // Add top
            measuredDepthValuesAdjusted.push_back( m_indexDepthOffsets->getTopMd( kLayer ) );
            valuesAdjusted.push_back( values[firstIndex] );

            // Add bottom of layer
            measuredDepthValuesAdjusted.push_back( m_indexDepthOffsets->getBottomMd( kLayer ) );
            valuesAdjusted.push_back( values[lastIndex] );
        }
    }

    return std::make_pair( valuesAdjusted, measuredDepthValuesAdjusted );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogLasFileCurve::setWellPath( RimWellPath* wellPath )
{
    m_wellPath = wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RimWellLogLasFileCurve::wellPath() const
{
    return m_wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogLasFileCurve::setWellLogChannelName( const QString& name )
{
    m_wellLogChannelName = name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogLasFileCurve::setWellLog( RimWellLog* wellLogFile )
{
    m_wellLog = wellLogFile;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogLasFileCurve::setIndexDepthOffsets( std::shared_ptr<RigWellLogIndexDepthOffset> depthOffsets )
{
    m_indexDepthOffsets = depthOffsets;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogLasFileCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimWellLogCurve::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_wellPath )
    {
        loadDataAndUpdate( true );
    }
    else if ( changedField == &m_wellLogChannelName )
    {
        loadDataAndUpdate( true );
    }
    else if ( changedField == &m_wellLog )
    {
        loadDataAndUpdate( true );
    }
    if ( m_parentPlot ) m_parentPlot->replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogLasFileCurve::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimPlotCurve::updateFieldUiState();

    caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup( "Curve Data" );
    curveDataGroup->add( &m_wellPath );
    curveDataGroup->add( &m_wellLog );
    curveDataGroup->add( &m_wellLogChannelName );

    RimStackablePlotCurve::defaultUiOrdering( uiOrdering );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogLasFileCurve::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellLogLasFileCurve::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    options = RimWellLogCurve::calculateValueOptions( fieldNeedingOptions );
    if ( !options.empty() ) return options;

    if ( fieldNeedingOptions == &m_wellPath )
    {
        auto wellPathColl = RimTools::wellPathCollection();
        if ( wellPathColl )
        {
            for ( auto wellPath : wellPathColl->allWellPaths() )
            {
                // Only include well paths coming with a well log
                if ( !wellPath->wellLogs().empty() )
                {
                    options.push_back( caf::PdmOptionItemInfo( wellPath->name(), wellPath ) );
                }
            }

            if ( !options.empty() )
            {
                options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
            }
        }
    }

    if ( fieldNeedingOptions == &m_wellLogChannelName )
    {
        if ( m_wellPath() && m_wellLog() )
        {
            for ( RimWellLogChannel* wellLogChannel : m_wellLog->wellLogChannels() )
            {
                QString wellLogChannelName = wellLogChannel->name();
                options.push_back( caf::PdmOptionItemInfo( wellLogChannelName, wellLogChannelName ) );
            }
        }

        if ( options.empty() )
        {
            options.push_back( caf::PdmOptionItemInfo( "None", "None" ) );
        }
    }

    if ( fieldNeedingOptions == &m_wellLog )
    {
        if ( m_wellPath() && !m_wellPath->wellLogs().empty() )
        {
            for ( RimWellLog* const wellLog : m_wellPath->wellLogs() )
            {
                if ( RimWellLogFile* wellLogFile = dynamic_cast<RimWellLogFile*>( wellLog ) )
                {
                    QFileInfo fileInfo( wellLogFile->fileName() );
                    options.push_back( caf::PdmOptionItemInfo( fileInfo.baseName(), wellLog ) );
                }
                else
                {
                    options.push_back( caf::PdmOptionItemInfo( wellLog->name(), wellLog ) );
                }
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogLasFileCurve::initAfterRead()
{
    RimWellLogCurve::initAfterRead();

    if ( !m_wellPath ) return;

    if ( m_wellPath->wellLogFiles().size() == 1 )
    {
        m_wellLog = dynamic_cast<RimWellLogLasFile*>( m_wellPath->wellLogFiles().front() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogLasFileCurve::isRftPlotChild() const
{
    RimWellRftPlot* rftPlot = firstAncestorOrThisOfType<RimWellRftPlot>();
    return rftPlot != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogLasFileCurve::createCurveAutoName()
{
    QStringList name;
    QString     unit;
    bool        channelNameAvailable = false;

    if ( m_wellPath )
    {
        name.push_back( wellName() );
        name.push_back( "LAS" );

        if ( !m_wellLogChannelName().isEmpty() )
        {
            name.push_back( m_wellLogChannelName );
            channelNameAvailable = true;
        }

        RigWellLogData* wellLogData = m_wellLog ? m_wellLog->wellLogData() : nullptr;

        if ( wellLogData )
        {
            if ( channelNameAvailable )
            {
                auto    wellLogPlot = firstAncestorOrThisOfTypeAsserted<RimWellLogPlot>();
                QString unitName    = wellLogData->convertedWellLogChannelUnitString( m_wellLogChannelName, wellLogPlot->depthUnit() );

                if ( !unitName.isEmpty() )
                {
                    name.back() += QString( " [%1]" ).arg( unitName );
                }
            }

            QString date = m_wellLog->date().toString( RiaQDateTimeTools::dateFormatString() );
            if ( !date.isEmpty() )
            {
                name.push_back( date );
            }
        }

        return name.join( ", " );
    }

    return "Empty curve";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogLasFileCurve::wellLogChannelUiName() const
{
    return m_wellLogChannelName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogLasFileCurve::wellLogChannelUnits() const
{
    if ( m_wellLog && m_wellLog->wellLogData() )
    {
        return m_wellLog->wellLogData()->wellLogChannelUnitString( m_wellLogChannelName );
    }
    return RiaWellLogUnitTools<double>::noUnitString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLog* RimWellLogLasFileCurve::wellLog() const
{
    return m_wellLog();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogLasFileCurve::wellName() const
{
    if ( m_wellPath )
    {
        return m_wellPath->name();
    }

    return QString( "" );
}
