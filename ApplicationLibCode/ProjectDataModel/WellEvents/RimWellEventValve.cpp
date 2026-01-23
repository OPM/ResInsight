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

#include "RimWellEventValve.h"

#include "RimProject.h"
#include "RimValveTemplate.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

namespace caf
{
template <>
void caf::AppEnum<RimWellEventValve::State>::setUp()
{
    addItem( RimWellEventValve::State::OPEN, "OPEN", "Open" );
    addItem( RimWellEventValve::State::SHUT, "SHUT", "Shut" );
    setDefault( RimWellEventValve::State::OPEN );
}

template <>
void caf::AppEnum<RimWellEventValve::ValveType>::setUp()
{
    addItem( RimWellEventValve::ValveType::ICV, "ICV", "ICV" );
    addItem( RimWellEventValve::ValveType::ICD, "ICD", "ICD" );
    addItem( RimWellEventValve::ValveType::AICD, "AICD", "AICD" );
    setDefault( RimWellEventValve::ValveType::ICV );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimWellEventValve, "WellEventValve" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventValve::RimWellEventValve()
{
    CAF_PDM_InitScriptableObject( "Valve Event", ":/ICVValve16x16.png", "", "WellEventValve" );

    CAF_PDM_InitScriptableField( &m_measuredDepth, "MeasuredDepth", 0.0, "Measured Depth" );
    CAF_PDM_InitScriptableField( &m_valveType, "ValveType", caf::AppEnum<ValveType>( ValveType::ICV ), "Valve Type" );
    CAF_PDM_InitScriptableField( &m_state, "State", caf::AppEnum<State>( State::OPEN ), "State" );
    CAF_PDM_InitScriptableField( &m_flowCoefficient, "FlowCoefficient", 0.7, "Flow Coefficient" );
    CAF_PDM_InitScriptableField( &m_area, "Area", 0.0001, "Area [m2]" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_valveTemplate, "ValveTemplate", "Valve Template" );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventValve::~RimWellEventValve()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellEventValve::measuredDepth() const
{
    return m_measuredDepth();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventValve::ValveType RimWellEventValve::valveType() const
{
    return m_valveType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventValve::State RimWellEventValve::state() const
{
    return m_state();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellEventValve::flowCoefficient() const
{
    if ( m_valveTemplate() )
    {
        return m_valveTemplate()->flowCoefficient();
    }
    return m_flowCoefficient();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellEventValve::area() const
{
    return m_area();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimValveTemplate* RimWellEventValve::valveTemplate() const
{
    return m_valveTemplate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventValve::setMeasuredDepth( double md )
{
    m_measuredDepth = md;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventValve::setValveType( ValveType type )
{
    m_valveType = type;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventValve::setState( State state )
{
    m_state = state;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventValve::setFlowCoefficient( double coefficient )
{
    m_flowCoefficient = coefficient;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventValve::setArea( double area )
{
    m_area = area;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventValve::setValveTemplate( RimValveTemplate* valveTemplate )
{
    m_valveTemplate = valveTemplate;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEvent::EventType RimWellEventValve::eventType() const
{
    return EventType::VALVE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellEventValve::generateScheduleKeyword( const QString& wellName ) const
{
    // Generate WSEGVALV keyword line for valve events
    // WSEGVALV format: Well Seg CV Ac
    QString line = QString( "-- %1 %2 at MD: %3, CV: %4, Area: %5, %6\n" )
                       .arg( wellName )
                       .arg( m_valveType().text() )
                       .arg( m_measuredDepth(), 0, 'f', 2 )
                       .arg( flowCoefficient(), 0, 'f', 3 )
                       .arg( m_area(), 0, 'e', 3 )
                       .arg( m_state().text() );

    return line;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventValve::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimWellEvent::defineUiOrdering( uiConfigName, uiOrdering );

    uiOrdering.add( &m_valveType );
    uiOrdering.add( &m_measuredDepth );
    uiOrdering.add( &m_state );
    uiOrdering.add( &m_valveTemplate );

    bool hasTemplate = m_valveTemplate() != nullptr;
    m_flowCoefficient.uiCapability()->setUiReadOnly( hasTemplate );

    uiOrdering.add( &m_flowCoefficient );
    uiOrdering.add( &m_area );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventValve::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    QString stateStr = ( m_state() == State::OPEN ) ? "OPEN" : "SHUT";
    QString valveTypeStr;
    switch ( m_valveType() )
    {
        case ValveType::ICV:
            valveTypeStr = "ICV";
            setUiIconFromResourceString( ":/ICVValve16x16.png" );
            break;
        case ValveType::ICD:
            valveTypeStr = "ICD";
            setUiIconFromResourceString( ":/ICDValve16x16.png" );
            break;
        case ValveType::AICD:
            valveTypeStr = "AICD";
            setUiIconFromResourceString( ":/AICDValve16x16.png" );
            break;
    }

    setUiName( QString( "%1 %2: MD %3 [%4]" )
                   .arg( valveTypeStr )
                   .arg( m_eventDate().toString( "yyyy-MM-dd" ) )
                   .arg( m_measuredDepth(), 0, 'f', 1 )
                   .arg( stateStr ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellEventValve::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_valveTemplate )
    {
        options.push_back( caf::PdmOptionItemInfo( "None", nullptr ) );

        RimProject* project = RimProject::current();
        if ( project )
        {
            std::vector<RimValveTemplate*> allTemplates = project->allValveTemplates();
            for ( RimValveTemplate* valveTemplate : allTemplates )
            {
                options.push_back( caf::PdmOptionItemInfo( valveTemplate->name(), valveTemplate ) );
            }
        }
    }

    return options;
}
