/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RimJobWellSettings.h"

#include "RimKeywordWconinje.h"
#include "RimKeywordWconprod.h"
#include "RimTools.h"
#include "RimWellPath.h"

#include "cafPdmFieldCapability.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiComboBoxEditor.h"

CAF_PDM_SOURCE_INIT( RimJobWellSettings, "JobWellSettings" );

namespace caf
{
template <>
void caf::AppEnum<RimJobWellSettings::WellOpenType>::setUp()
{
    addItem( RimJobWellSettings::WellOpenType::OPEN_BY_POSITION, "OpenByPosition", "By Position in File" );
    addItem( RimJobWellSettings::WellOpenType::OPEN_AT_DATE, "AtSelectedDate", "By Date" );

    setDefault( RimJobWellSettings::WellOpenType::OPEN_AT_DATE );
}

} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimJobWellSettings::RimJobWellSettings()
{
    CAF_PDM_InitObject( "Job Well Settings" );

    CAF_PDM_InitFieldNoDefault( &m_wellPath, "WellPath", "Well Path for New Well" );

    CAF_PDM_InitField( &m_addNewWell, "AddNewWell", true, "Add New Well" );
    CAF_PDM_InitField( &m_includeMSWData, "IncludeMswData", false, "Include MSW Data" );

    CAF_PDM_InitFieldNoDefault( &m_wellGroupName, "WellGroupName", "Well Group Name" );
    m_wellGroupName.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_wellOpenType, "WellOpenType", caf::AppEnum<WellOpenType>( WellOpenType::OPEN_AT_DATE ), "Open Well" );
    CAF_PDM_InitField( &m_wellOpenKeyword, "WellOpenKeyword", QString( "WCONPROD" ), "Open Well Keyword" );
    m_wellOpenKeyword.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    m_wellOpenKeyword.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_wconprodKeyword, "WconprodKeyword", "WCONPROD Settings" );
    m_wconprodKeyword = new RimKeywordWconprod();
    m_wconprodKeyword.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_wconinjeKeyword, "WconinjeKeyword", "WCONINJE Settings" );
    m_wconinjeKeyword = new RimKeywordWconinje();
    m_wconinjeKeyword.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitField( &m_openTimeStep, "OpenTimeStep", 0, " " );

    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_includeMSWData );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_addNewWell );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimJobWellSettings::~RimJobWellSettings()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimJobWellSettings::setWellPath( RimWellPath* wellPath )
{
    m_wellPath = wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimJobWellSettings::setWellGroupName( const QString& wellGroupName )
{
    m_wellGroupName = wellGroupName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimJobWellSettings::setOpenTimeStep( int openTimeStep )
{
    m_openTimeStep = openTimeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimJobWellSettings::setWellOpenType( WellOpenType wellOpenType )
{
    m_wellOpenType = caf::AppEnum<WellOpenType>( wellOpenType );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimJobWellSettings::setWellOpenKeyword( const QString& wellOpenKeyword )
{
    m_wellOpenKeyword = wellOpenKeyword;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimJobWellSettings::setWconprodKeyword( RimKeywordWconprod* wconprodKeyword )
{
    m_wconprodKeyword = wconprodKeyword;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimJobWellSettings::setWconinjeKeyword( RimKeywordWconinje* wconinjeKeyword )
{
    m_wconinjeKeyword = wconinjeKeyword;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimJobWellSettings::setIncludeMSWData( bool includeMSWData )
{
    m_includeMSWData = includeMSWData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimJobWellSettings::addNewWell() const
{
    return m_addNewWell;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RimJobWellSettings::wellPath() const
{
    return m_wellPath();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimJobWellSettings::wellGroupName() const
{
    return m_wellGroupName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimJobWellSettings::openTimeStep() const
{
    return m_openTimeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimJobWellSettings::WellOpenType RimJobWellSettings::wellOpenType() const
{
    return m_wellOpenType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimJobWellSettings::wellOpenKeyword() const
{
    return m_wellOpenKeyword;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimKeywordWconprod* RimJobWellSettings::wconprodKeyword() const
{
    return m_wconprodKeyword();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimKeywordWconinje* RimJobWellSettings::wconinjeKeyword() const
{
    return m_wconinjeKeyword();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimJobWellSettings::includeMSWData() const
{
    return m_includeMSWData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimJobWellSettings::useDateStrings( const std::vector<QString>& dateStrings )
{
    m_dateStrings = dateStrings;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimJobWellSettings::useWellGroups( const std::vector<QString>& wellGroups )
{
    m_wellGroups = wellGroups;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimJobWellSettings::uiOrdering( caf::PdmUiGroup* uiGroup )
{
    auto wellGrp = uiGroup->addNewGroup( "New Well Settings" );
    wellGrp->add( &m_addNewWell );

    if ( m_addNewWell() )
    {
        wellGrp->add( &m_wellPath );
        wellGrp->add( &m_wellGroupName );
        wellGrp->add( &m_wellOpenKeyword );
        if ( m_wellOpenKeyword() == "WCONPROD" )
        {
            auto wconGrp = wellGrp->addNewGroup( "WCONPROD Settings" );
            m_wconprodKeyword->uiOrdering( wconGrp );
            wconGrp->setCollapsedByDefault();
        }
        else
        {
            auto wconGrp = wellGrp->addNewGroup( "WCONINJE Settings" );
            m_wconinjeKeyword->uiOrdering( wconGrp );
            wconGrp->setCollapsedByDefault();
        }

        wellGrp->add( &m_wellOpenType );

        if ( m_wellOpenType() == WellOpenType::OPEN_AT_DATE )
        {
            wellGrp->add( &m_openTimeStep );
        }
        m_wellOpenType.uiCapability()->setUiReadOnly( false );

        wellGrp->add( &m_includeMSWData );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimJobWellSettings::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_wellPath )
    {
        RimTools::wellPathOptionItems( &options );
    }
    else if ( ( fieldNeedingOptions == &m_openTimeStep ) )
    {
        auto timeStepNames = m_dateStrings;
        for ( int i = 0; i < static_cast<int>( timeStepNames.size() - 1 ); i++ )
        {
            options.push_back( caf::PdmOptionItemInfo( timeStepNames[i], QVariant::fromValue( i ) ) );
        }
    }
    else if ( fieldNeedingOptions == &m_wellOpenKeyword )
    {
        options.push_back( caf::PdmOptionItemInfo( "WCONPROD", QVariant::fromValue( QString( "WCONPROD" ) ) ) );
        options.push_back( caf::PdmOptionItemInfo( "WCONINJE", QVariant::fromValue( QString( "WCONINJE" ) ) ) );
    }
    else if ( fieldNeedingOptions == &m_wellGroupName )
    {
        for ( auto& grp : m_wellGroups )
        {
            options.push_back( caf::PdmOptionItemInfo( grp, QVariant::fromValue( grp ) ) );
        }
    }

    return options;
}
