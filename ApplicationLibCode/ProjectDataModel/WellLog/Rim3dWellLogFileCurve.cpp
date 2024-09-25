/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "Rim3dWellLogFileCurve.h"

#include "RigWellLogLasFile.h"

#include "RimWellLogChannel.h"
#include "RimWellLogLasFile.h"
#include "RimWellLogLasFileCurveNameConfig.h"
#include "RimWellPath.h"

#include "RiaQDateTimeTools.h"

#include <QFileInfo>

//==================================================================================================
///
///
//==================================================================================================

CAF_PDM_SOURCE_INIT( Rim3dWellLogFileCurve, "Rim3dWellLogFileCurve" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dWellLogFileCurve::Rim3dWellLogFileCurve()
{
    CAF_PDM_InitObject( "3d Well Log File Curve", ":/WellLogCurve16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_wellLogChannelName, "CurveWellLogChannel", "Well Log Channel" );

    CAF_PDM_InitFieldNoDefault( &m_wellLog, "WellLog", "Well Log" );
    m_wellLog.registerKeywordAlias( "WellLogFile" );

    CAF_PDM_InitFieldNoDefault( &m_nameConfig, "NameConfig", "" );
    m_nameConfig = new RimWellLogLasFileCurveNameConfig();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dWellLogFileCurve::~Rim3dWellLogFileCurve()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogFileCurve::setDefaultFileCurveDataInfo()
{
    auto wellPath = firstAncestorOrThisOfType<RimWellPath>();

    if ( wellPath && !wellPath->wellLogs().empty() )
    {
        m_wellLog = wellPath->wellLogs()[0];
    }

    if ( m_wellLog )
    {
        std::vector<RimWellLogChannel*> wellLogChannels = m_wellLog->wellLogChannels();
        if ( !wellLogChannels.empty() )
        {
            m_wellLogChannelName = wellLogChannels[0]->name();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogFileCurve::curveValuesAndMds( std::vector<double>* values, std::vector<double>* measuredDepthValues ) const
{
    CAF_ASSERT( values != nullptr );
    CAF_ASSERT( measuredDepthValues != nullptr );

    if ( m_wellLog )
    {
        RigWellLogData* wellLogData = m_wellLog->wellLogData();
        if ( wellLogData )
        {
            *values              = wellLogData->values( m_wellLogChannelName );
            *measuredDepthValues = wellLogData->depthValues();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Rim3dWellLogFileCurve::resultPropertyString() const
{
    return m_wellLogChannelName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Rim3dWellLogFileCurve::name() const
{
    return m_nameConfig->name();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Rim3dWellLogFileCurve::createAutoName() const
{
    QStringList name;
    QString     unit;
    bool        channelNameAvailable = false;

    auto wellPath = firstAncestorOrThisOfType<RimWellPath>();
    if ( wellPath )
    {
        name.push_back( wellPath->name() );
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
                /*   RimWellLogPlot* wellLogPlot;
                   firstAncestorOrThisOfType(wellLogPlot);
                   CVF_ASSERT(wellLogPlot);
                   QString unitName = wellLogFile->wellLogChannelUnitString(m_wellLogChannelName,
                   wellLogPlot->depthUnit());

                   if (!unitName.isEmpty())
                   {
                       name.back() += QString(" [%1]").arg(unitName);
                   } */
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
caf::PdmFieldHandle* Rim3dWellLogFileCurve::userDescriptionField()
{
    return m_nameConfig()->nameField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogFileCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_wellLog || changedField == &m_wellLogChannelName )
    {
        resetMinMaxValues();
        updateConnectedEditors();
    }
    Rim3dWellLogCurve::fieldChangedByUi( changedField, oldValue, newValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> Rim3dWellLogFileCurve::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options = Rim3dWellLogCurve::calculateValueOptions( fieldNeedingOptions );
    if ( !options.empty() ) return options;

    if ( fieldNeedingOptions == &m_wellLogChannelName )
    {
        if ( m_wellLog )
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
        auto wellPath = firstAncestorOrThisOfType<RimWellPath>();

        if ( wellPath )
        {
            for ( RimWellLog* const wellLog : wellPath->wellLogs() )
            {
                options.push_back( caf::PdmOptionItemInfo( wellLog->name(), wellLog ) );
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogFileCurve::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup( "Curve Data" );
    curveDataGroup->add( &m_wellLog );
    curveDataGroup->add( &m_wellLogChannelName );

    Rim3dWellLogCurve::configurationUiOrdering( uiOrdering );

    caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup( "Curve Name" );
    m_nameConfig->uiOrdering( uiConfigName, *nameGroup );

    uiOrdering.skipRemainingFields( true );
}
