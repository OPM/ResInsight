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

#include "RimWellLogFileCurve.h"

#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "RiaResultNames.h"
#include "RigWellLogCurveData.h"
#include "RigWellLogIndexDepthOffset.h"
#include "RigWellPath.h"

#include "RimProject.h"
#include "RimTools.h"
#include "RimWellLogFile.h"
#include "RimWellLogFileChannel.h"
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

CAF_PDM_SOURCE_INIT( RimWellLogFileCurve, "WellLogFileCurve" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogFileCurve::RimWellLogFileCurve()
{
    CAF_PDM_InitObject( "Well Log File Curve", RimWellLogCurve::wellLogCurveIconName(), "", "" );

    CAF_PDM_InitFieldNoDefault( &m_wellPath, "CurveWellPath", "Well Path", "", "", "" );
    m_wellPath.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_wellLogChannelName, "CurveWellLogChannel", "Well Log Channel", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_wellLogFile, "WellLogFile", "Well Log File", "", "", "" );

    m_wellPath = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogFileCurve::~RimWellLogFileCurve()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogFileCurve::onLoadDataAndUpdate( bool updateParentPlot )
{
    this->RimPlotCurve::updateCurvePresentation( updateParentPlot );

    if ( isCurveVisible() )
    {
        RimWellLogPlot* wellLogPlot;
        firstAncestorOrThisOfType( wellLogPlot );
        CVF_ASSERT( wellLogPlot );

        if ( m_wellPath && m_wellLogFile )
        {
            RigWellLogFile* wellLogFile = m_wellLogFile->wellLogFileData();
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
                    auto [valuesAdjusted, measuredDepthValuesAdjusted] =
                        adjustByIndexDepthOffsets( measuredDepthValues, values, kIndexValues );

                    values              = valuesAdjusted;
                    measuredDepthValues = measuredDepthValuesAdjusted;
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

                this->setValuesAndDepths( values, validDepths, rkbDiff, wellLogFile->depthUnit(), false );

                QString errMsg;
                if ( wellLogPlot && !this->curveData()->availableDepthTypes().count( wellLogPlot->depthType() ) )
                {
                    QString depthTitle = wellLogPlot->depthAxisTitle();
                    errMsg             = QString( "Display of %1 for LAS curves is not possible without %2 "
                                      "values in the LAS-file or a well path to derive them from." )
                                 .arg( depthTitle )
                                 .arg( depthTitle );
                }

                bool showWarning = !RiaPreferences::current()->showLasCurveWithoutTvdWarning();
                if ( !errMsg.isEmpty() && showWarning )
                {
                    QString tmp = QString( "The LAS curve can not be displayed.\n%1\n" ).arg( errMsg );
                    tmp += "Control display of this warning from \"Preferences->Show LAS curve without TVD "
                           "warning\"";

                    RiaLogging::errorInMessageBox( nullptr, "LAS curve without current depth type", tmp );
                }
            }

            if ( m_isUsingAutoName )
            {
                m_qwtPlotCurve->setTitle( createCurveAutoName() );
            }
        }

        RiaDefines::DepthUnitType displayUnit = RiaDefines::DepthUnitType::UNIT_METER;
        if ( wellLogPlot )
        {
            displayUnit = wellLogPlot->depthUnit();
        }

        RiaDefines::DepthTypeEnum depthType = RiaDefines::DepthTypeEnum::MEASURED_DEPTH;
        if ( wellLogPlot && this->curveData()->availableDepthTypes().count( wellLogPlot->depthType() ) )
        {
            depthType = wellLogPlot->depthType();
        }

        m_qwtPlotCurve->setSamples( this->curveData()->xPlotValues().data(),
                                    this->curveData()->depthPlotValues( depthType, displayUnit ).data(),
                                    static_cast<int>( this->curveData()->xPlotValues().size() ) );
        m_qwtPlotCurve->setLineSegmentStartStopIndices( this->curveData()->polylineStartStopIndices() );

        if ( updateParentPlot )
        {
            updateZoomInParentPlot();
        }

        if ( m_parentQwtPlot )
        {
            m_parentQwtPlot->replot();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<double>, std::vector<double>>
    RimWellLogFileCurve::adjustByIndexDepthOffsets( const std::vector<double>& measuredDepthValues,
                                                    const std::vector<double>& values,
                                                    const std::vector<double>& kIndexValues ) const
{
    CAF_ASSERT( values.size() == kIndexValues.size() );

    auto findFirstIndex = []( int kLayer, const std::vector<double>& vals ) {
        for ( size_t i = 0; i < vals.size(); i++ )
            if ( kLayer == static_cast<int>( vals[i] ) ) return i;

        return vals.size();
    };

    auto findLastIndex = []( int kLayer, const std::vector<double>& vals ) {
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
            measuredDepthValuesAdjusted.push_back( m_indexDepthOffsets->getTopDepth( kLayer ) );
            valuesAdjusted.push_back( values[firstIndex] );

            // Add bottom of layer
            measuredDepthValuesAdjusted.push_back( m_indexDepthOffsets->getBottomDepth( kLayer ) );
            valuesAdjusted.push_back( values[lastIndex] );
        }
    }

    return std::make_pair( valuesAdjusted, measuredDepthValuesAdjusted );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogFileCurve::setWellPath( RimWellPath* wellPath )
{
    m_wellPath = wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RimWellLogFileCurve::wellPath() const
{
    return m_wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogFileCurve::setWellLogChannelName( const QString& name )
{
    m_wellLogChannelName = name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogFileCurve::setWellLogFile( RimWellLogFile* wellLogFile )
{
    m_wellLogFile = wellLogFile;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogFileCurve::setIndexDepthOffsets( std::shared_ptr<RigWellLogIndexDepthOffset> depthOffsets )
{
    m_indexDepthOffsets = depthOffsets;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogFileCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                            const QVariant&            oldValue,
                                            const QVariant&            newValue )
{
    RimWellLogCurve::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_wellPath )
    {
        this->loadDataAndUpdate( true );
    }
    else if ( changedField == &m_wellLogChannelName )
    {
        this->loadDataAndUpdate( true );
    }
    else if ( changedField == &m_wellLogFile )
    {
        this->loadDataAndUpdate( true );
    }
    if ( m_parentQwtPlot ) m_parentQwtPlot->replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogFileCurve::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimPlotCurve::updateOptionSensitivity();

    caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup( "Curve Data" );
    curveDataGroup->add( &m_wellPath );
    curveDataGroup->add( &m_wellLogFile );
    curveDataGroup->add( &m_wellLogChannelName );

    caf::PdmUiGroup* stackingGroup = uiOrdering.addNewGroup( "Stacking" );
    RimStackablePlotCurve::stackingUiOrdering( *stackingGroup );

    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup( "Appearance" );
    RimPlotCurve::appearanceUiOrdering( *appearanceGroup );

    caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup( "Curve Name" );
    nameGroup->add( &m_showLegend );
    RimPlotCurve::curveNameUiOrdering( *nameGroup );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogFileCurve::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellLogFileCurve::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                          bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    options = RimWellLogCurve::calculateValueOptions( fieldNeedingOptions, useOptionsOnly );
    if ( options.size() > 0 ) return options;

    if ( fieldNeedingOptions == &m_wellPath )
    {
        auto wellPathColl = RimTools::wellPathCollection();
        if ( wellPathColl )
        {
            for ( auto wellPath : wellPathColl->allWellPaths() )
            {
                // Only include well paths coming from a well log file
                if ( wellPath->wellLogFiles().size() > 0 )
                {
                    options.push_back( caf::PdmOptionItemInfo( wellPath->name(), wellPath ) );
                }
            }

            if ( options.size() > 0 )
            {
                options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
            }
        }
    }

    if ( fieldNeedingOptions == &m_wellLogChannelName )
    {
        if ( m_wellPath() )
        {
            if ( m_wellLogFile )
            {
                std::vector<RimWellLogFileChannel*> fileLogs = m_wellLogFile->wellLogChannels();

                for ( size_t i = 0; i < fileLogs.size(); i++ )
                {
                    QString wellLogChannelName = fileLogs[i]->name();
                    options.push_back( caf::PdmOptionItemInfo( wellLogChannelName, wellLogChannelName ) );
                }
            }
        }

        if ( options.size() == 0 )
        {
            options.push_back( caf::PdmOptionItemInfo( "None", "None" ) );
        }
    }

    if ( fieldNeedingOptions == &m_wellLogFile )
    {
        if ( m_wellPath() && m_wellPath->wellLogFiles().size() > 0 )
        {
            for ( RimWellLogFile* const wellLogFile : m_wellPath->wellLogFiles() )
            {
                QFileInfo fileInfo( wellLogFile->fileName() );
                options.push_back( caf::PdmOptionItemInfo( fileInfo.baseName(), wellLogFile ) );
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogFileCurve::initAfterRead()
{
    if ( !m_wellPath ) return;

    if ( m_wellPath->wellLogFiles().size() == 1 )
    {
        m_wellLogFile = m_wellPath->wellLogFiles().front();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogFileCurve::isRftPlotChild() const
{
    RimWellRftPlot* rftPlot;
    firstAncestorOrThisOfType( rftPlot );
    return rftPlot != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogFileCurve::createCurveAutoName()
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

        RigWellLogFile* wellLogFile = m_wellLogFile ? m_wellLogFile->wellLogFileData() : nullptr;

        if ( wellLogFile )
        {
            if ( channelNameAvailable )
            {
                RimWellLogPlot* wellLogPlot;
                firstAncestorOrThisOfType( wellLogPlot );
                CVF_ASSERT( wellLogPlot );
                QString unitName = wellLogFile->wellLogChannelUnitString( m_wellLogChannelName, wellLogPlot->depthUnit() );

                if ( !unitName.isEmpty() )
                {
                    name.back() += QString( " [%1]" ).arg( unitName );
                }
            }

            QString date = wellLogFile->date();
            if ( !date.isEmpty() )
            {
                name.push_back( wellLogFile->date() );
            }
        }

        return name.join( ", " );
    }

    return "Empty curve";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogFileCurve::wellLogChannelUiName() const
{
    return m_wellLogChannelName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogFileCurve::wellLogChannelUnits() const
{
    if ( m_wellLogFile && m_wellLogFile->wellLogFileData() )
    {
        return m_wellLogFile->wellLogFileData()->wellLogChannelUnitString( m_wellLogChannelName );
    }
    return RiaWellLogUnitTools<double>::noUnitString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogFile* RimWellLogFileCurve::wellLogFile() const
{
    return m_wellLogFile();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogFileCurve::wellName() const
{
    if ( m_wellPath )
    {
        return m_wellPath->name();
    }

    return QString( "" );
}
