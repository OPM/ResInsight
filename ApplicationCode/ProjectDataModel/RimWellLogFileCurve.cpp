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

#include "RigWellLogCurveData.h"
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

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "cafPdmUiTreeOrdering.h"

#include <QFileInfo>
#include <QMessageBox>

CAF_PDM_SOURCE_INIT( RimWellLogFileCurve, "WellLogFileCurve" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogFileCurve::RimWellLogFileCurve()
{
    CAF_PDM_InitObject( "Well Log File Curve", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_wellPath, "CurveWellPath", "Well Path", "", "", "" );
    m_wellPath.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_wellLogChannnelName, "CurveWellLogChannel", "Well Log Channel", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_wellLogFile, "WellLogFile", "Well Log File", "", "", "" );

    m_wellPath = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogFileCurve::~RimWellLogFileCurve() {}

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
                std::vector<double> values              = wellLogFile->values( m_wellLogChannnelName );
                std::vector<double> measuredDepthValues = wellLogFile->depthValues();
                std::vector<double> tvdMslValues        = wellLogFile->tvdMslValues();
                std::vector<double> tvdRkbValues        = wellLogFile->tvdRkbValues();

                bool rkbDiff = m_wellPath->wellPathGeometry() ? m_wellPath->wellPathGeometry()->rkbDiff() : 0.0;

                if ( tvdMslValues.size() != values.size() )
                {
                    RigWellPath* rigWellPath = m_wellPath->wellPathGeometry();
                    if ( rigWellPath )
                    {
                        tvdMslValues.clear();
                        for ( double measuredDepthValue : measuredDepthValues )
                        {
                            tvdMslValues.push_back(
                                -rigWellPath->interpolatedPointAlongWellPath( measuredDepthValue ).z() );
                        }
                    }
                }

                std::map<RiaDefines::DepthTypeEnum, std::vector<double>> validDepths;
                if ( values.size() == measuredDepthValues.size() )
                {
                    validDepths.insert( std::make_pair( RiaDefines::MEASURED_DEPTH, measuredDepthValues ) );
                }
                if ( values.size() == tvdMslValues.size() )
                {
                    validDepths.insert( std::make_pair( RiaDefines::TRUE_VERTICAL_DEPTH, tvdMslValues ) );
                }
                if ( values.size() == tvdRkbValues.size() )
                {
                    validDepths.insert( std::make_pair( RiaDefines::TRUE_VERTICAL_DEPTH_RKB, tvdRkbValues ) );
                }

                this->setValuesAndDepths( values, validDepths, rkbDiff, wellLogFile->depthUnit(), false );

                QString errMsg;
                if ( wellLogPlot && !this->curveData()->availableDepthTypes().count( wellLogPlot->depthType() ) )
                {
                    QString depthTitle = wellLogPlot->depthAxisTitle();
                    errMsg             = QString( "Display of %1 for LAS curves is not possible without %1 "
                                      "values in the LAS-file or a well path to derive them from." )
                                 .arg( depthTitle )
                                 .arg( depthTitle );
                }

                bool showWarning = !RiaApplication::instance()->preferences()->showLasCurveWithoutTvdWarning();
                if ( !errMsg.isEmpty() && showWarning )
                {
                    QString tmp = QString( "The LAS curve can not be displayed.\n%1\n" ).arg( errMsg );
                    tmp += "Control display of this warning from \"Preferences->Show LAS curve without TVD "
                           "warning\"";

                    QMessageBox::warning( nullptr, "LAS curve without current depth type", tmp );
                }
            }

            if ( m_isUsingAutoName )
            {
                m_qwtPlotCurve->setTitle( createCurveAutoName() );
            }
        }

        RiaDefines::DepthUnitType displayUnit = RiaDefines::UNIT_METER;
        if ( wellLogPlot )
        {
            displayUnit = wellLogPlot->depthUnit();
        }

        RiaDefines::DepthTypeEnum depthType = RiaDefines::MEASURED_DEPTH;
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
    m_wellLogChannnelName = name;
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
void RimWellLogFileCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                            const QVariant&            oldValue,
                                            const QVariant&            newValue )
{
    RimWellLogCurve::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_wellPath )
    {
        this->loadDataAndUpdate( true );
    }
    else if ( changedField == &m_wellLogChannnelName )
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
    curveDataGroup->add( &m_wellLogChannnelName );

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
            caf::PdmChildArrayField<RimWellPath*>& wellPaths = wellPathColl->wellPaths;

            for ( size_t i = 0; i < wellPaths.size(); i++ )
            {
                // Only include well paths coming from a well log file
                if ( wellPaths[i]->wellLogFiles().size() > 0 )
                {
                    options.push_back( caf::PdmOptionItemInfo( wellPaths[i]->name(), wellPaths[i] ) );
                }
            }

            if ( options.size() > 0 )
            {
                options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
            }
        }
    }

    if ( fieldNeedingOptions == &m_wellLogChannnelName )
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

        if ( !m_wellLogChannnelName().isEmpty() )
        {
            name.push_back( m_wellLogChannnelName );
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
                QString unitName = wellLogFile->wellLogChannelUnitString( m_wellLogChannnelName,
                                                                          wellLogPlot->depthUnit() );

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
QString RimWellLogFileCurve::wellLogChannelName() const
{
    return m_wellLogChannnelName;
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
