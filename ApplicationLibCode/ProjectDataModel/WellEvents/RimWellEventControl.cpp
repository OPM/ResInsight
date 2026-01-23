/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

#include "RimWellEventControl.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

namespace caf
{
template <>
void caf::AppEnum<RimWellEventControl::ControlMode>::setUp()
{
    addItem( RimWellEventControl::ControlMode::ORAT, "ORAT", "Oil Rate" );
    addItem( RimWellEventControl::ControlMode::WRAT, "WRAT", "Water Rate" );
    addItem( RimWellEventControl::ControlMode::GRAT, "GRAT", "Gas Rate" );
    addItem( RimWellEventControl::ControlMode::LRAT, "LRAT", "Liquid Rate" );
    addItem( RimWellEventControl::ControlMode::RESV, "RESV", "Reservoir Volume" );
    addItem( RimWellEventControl::ControlMode::BHP, "BHP", "BHP" );
    addItem( RimWellEventControl::ControlMode::THP, "THP", "THP" );
    setDefault( RimWellEventControl::ControlMode::ORAT );
}

template <>
void caf::AppEnum<RimWellEventControl::WellStatus>::setUp()
{
    addItem( RimWellEventControl::WellStatus::OPEN, "OPEN", "Open" );
    addItem( RimWellEventControl::WellStatus::SHUT, "SHUT", "Shut" );
    addItem( RimWellEventControl::WellStatus::STOP, "STOP", "Stop" );
    setDefault( RimWellEventControl::WellStatus::OPEN );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimWellEventControl, "WellEventControl" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventControl::RimWellEventControl()
{
    CAF_PDM_InitScriptableObject( "Well Control Event", "", "", "WellEventControl" );

    CAF_PDM_InitScriptableField( &m_controlMode, "ControlMode", caf::AppEnum<ControlMode>( ControlMode::ORAT ), "Control Mode" );
    CAF_PDM_InitScriptableField( &m_wellStatus, "WellStatus", caf::AppEnum<WellStatus>( WellStatus::OPEN ), "Well Status" );
    CAF_PDM_InitScriptableField( &m_controlValue, "ControlValue", 0.0, "Control Value" );
    CAF_PDM_InitScriptableField( &m_bhpLimit, "BhpLimit", 0.0, "BHP Limit [bar]" );
    CAF_PDM_InitScriptableField( &m_thpLimit, "ThpLimit", 0.0, "THP Limit [bar]" );
    CAF_PDM_InitScriptableField( &m_oilRate, "OilRate", 0.0, "Oil Rate [Sm3/day]" );
    CAF_PDM_InitScriptableField( &m_waterRate, "WaterRate", 0.0, "Water Rate [Sm3/day]" );
    CAF_PDM_InitScriptableField( &m_gasRate, "GasRate", 0.0, "Gas Rate [Sm3/day]" );
    CAF_PDM_InitScriptableField( &m_liquidRate, "LiquidRate", 0.0, "Liquid Rate [Sm3/day]" );
    CAF_PDM_InitScriptableField( &m_isProducer, "IsProducer", true, "Is Producer" );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventControl::~RimWellEventControl()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventControl::ControlMode RimWellEventControl::controlMode() const
{
    return m_controlMode();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventControl::WellStatus RimWellEventControl::wellStatus() const
{
    return m_wellStatus();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellEventControl::controlValue() const
{
    return m_controlValue();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellEventControl::bhpLimit() const
{
    return m_bhpLimit();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellEventControl::thpLimit() const
{
    return m_thpLimit();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellEventControl::oilRate() const
{
    return m_oilRate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellEventControl::waterRate() const
{
    return m_waterRate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellEventControl::gasRate() const
{
    return m_gasRate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellEventControl::liquidRate() const
{
    return m_liquidRate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellEventControl::isProducer() const
{
    return m_isProducer();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventControl::setControlMode( ControlMode mode )
{
    m_controlMode = mode;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventControl::setWellStatus( WellStatus status )
{
    m_wellStatus = status;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventControl::setControlValue( double value )
{
    m_controlValue = value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventControl::setBhpLimit( double limit )
{
    m_bhpLimit = limit;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventControl::setThpLimit( double limit )
{
    m_thpLimit = limit;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventControl::setOilRate( double rate )
{
    m_oilRate = rate;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventControl::setWaterRate( double rate )
{
    m_waterRate = rate;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventControl::setGasRate( double rate )
{
    m_gasRate = rate;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventControl::setLiquidRate( double rate )
{
    m_liquidRate = rate;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventControl::setIsProducer( bool isProducer )
{
    m_isProducer = isProducer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEvent::EventType RimWellEventControl::eventType() const
{
    return EventType::WCONTROL;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellEventControl::generateScheduleKeyword( const QString& wellName ) const
{
    if ( m_isProducer() )
    {
        return generateWCONPROD( wellName );
    }
    else
    {
        // Determine injector type based on control mode or use water as default
        QString injectorType = "WATER";
        if ( m_controlMode() == ControlMode::GRAT )
        {
            injectorType = "GAS";
        }
        return generateWCONINJE( wellName, injectorType );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellEventControl::generateWCONPROD( const QString& wellName ) const
{
    // WCONPROD format:
    // Well Status Ctrl OilRate WatRate GasRate LiqRate ResV BHP THP
    auto formatValue = []( double val ) -> QString
    {
        if ( val == 0.0 ) return "1*";
        return QString::number( val, 'f', 1 );
    };

    QString line = QString( "WCONPROD\n" );
    line += QString( "-- Well   Status Ctrl  OilRate  WatRate  GasRate  LiqRate  ResV      BHP     THP\n" );
    line += QString( "   %1  %2   %3   %4   %5   %6   %7   1*   %8   %9 /\n" )
                .arg( wellName, -8 )
                .arg( m_wellStatus().text(), -4 )
                .arg( m_controlMode().text(), -4 )
                .arg( formatValue( m_oilRate() ), 8 )
                .arg( formatValue( m_waterRate() ), 8 )
                .arg( formatValue( m_gasRate() ), 8 )
                .arg( formatValue( m_liquidRate() ), 8 )
                .arg( formatValue( m_bhpLimit() ), 6 )
                .arg( formatValue( m_thpLimit() ), 6 );
    line += "/\n\n";

    return line;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellEventControl::generateWCONINJE( const QString& wellName, const QString& injectorType ) const
{
    // WCONINJE format:
    // Well Type Status Ctrl Rate BHP
    QString ctrlStr;
    switch ( m_controlMode() )
    {
        case ControlMode::WRAT:
            ctrlStr = "RATE";
            break;
        case ControlMode::GRAT:
            ctrlStr = "RATE";
            break;
        case ControlMode::BHP:
            ctrlStr = "BHP";
            break;
        case ControlMode::THP:
            ctrlStr = "THP";
            break;
        default:
            ctrlStr = "RATE";
            break;
    }

    auto formatValue = []( double val ) -> QString
    {
        if ( val == 0.0 ) return "1*";
        return QString::number( val, 'f', 1 );
    };

    double rate = ( injectorType == "WATER" ) ? m_waterRate() : m_gasRate();

    QString line = QString( "WCONINJE\n" );
    line += QString( "-- Well   Type   Status Ctrl   Rate         BHP\n" );
    line += QString( "   %1  %2  %3   %4   %5   %6 /\n" )
                .arg( wellName, -8 )
                .arg( injectorType, -5 )
                .arg( m_wellStatus().text(), -4 )
                .arg( ctrlStr, -4 )
                .arg( formatValue( rate ), 10 )
                .arg( formatValue( m_bhpLimit() ), 8 );
    line += "/\n\n";

    return line;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventControl::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimWellEvent::defineUiOrdering( uiConfigName, uiOrdering );

    uiOrdering.add( &m_isProducer );
    uiOrdering.add( &m_wellStatus );
    uiOrdering.add( &m_controlMode );
    uiOrdering.add( &m_controlValue );

    caf::PdmUiGroup* ratesGroup = uiOrdering.addNewGroup( "Rate Limits" );
    ratesGroup->add( &m_oilRate );
    ratesGroup->add( &m_waterRate );
    ratesGroup->add( &m_gasRate );
    ratesGroup->add( &m_liquidRate );

    caf::PdmUiGroup* pressureGroup = uiOrdering.addNewGroup( "Pressure Limits" );
    pressureGroup->add( &m_bhpLimit );
    pressureGroup->add( &m_thpLimit );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventControl::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    QString typeStr  = m_isProducer() ? "PROD" : "INJ";
    QString ctrlStr  = m_controlMode().uiText();
    QString valueStr = QString::number( m_controlValue(), 'f', 0 );

    setUiName( QString( "WCTRL %1: %2 %3 %4" ).arg( m_eventDate().toString( "yyyy-MM-dd" ) ).arg( typeStr ).arg( ctrlStr ).arg( valueStr ) );
}
