/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RimSummarySumoDataSource.h"

#include "RiaStdStringTools.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiTreeSelectionEditor.h"

CAF_PDM_SOURCE_INIT( RimSummarySumoDataSource, "RimSummarySumoDataSource" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummarySumoDataSource::RimSummarySumoDataSource()
{
    CAF_PDM_InitObject( "Sumo Data Source", ":/CloudBlobs.svg" );

    CAF_PDM_InitFieldNoDefault( &m_caseId, "CaseId", "Case Id" );
    CAF_PDM_InitFieldNoDefault( &m_caseName, "CaseName", "Case Name" );
    CAF_PDM_InitFieldNoDefault( &m_ensembleName, "EnsembleName", "Ensemble Name" );
    CAF_PDM_InitFieldNoDefault( &m_customName, "CustomName", "Custom Name" );

    CAF_PDM_InitFieldNoDefault( &m_realizationIds, "RealizationIds", "Realizations Ids" );
    m_realizationIds.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_realizationInfo, "NameProxy", "Realization Info" );
    m_realizationInfo.registerGetMethod( this, &RimSummarySumoDataSource::realizationInfoText );

    CAF_PDM_InitFieldNoDefault( &m_vectorNames, "VectorNames", "Vector Names" );
    m_vectorNames.uiCapability()->setUiReadOnly( true );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
SumoCaseId RimSummarySumoDataSource::caseId() const
{
    return SumoCaseId( m_caseId() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummarySumoDataSource::setCaseId( const SumoCaseId& caseId )
{
    m_caseId = caseId.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummarySumoDataSource::caseName() const
{
    return m_caseName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummarySumoDataSource::setCaseName( const QString& caseName )
{
    m_caseName = caseName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummarySumoDataSource::ensembleName() const
{
    return m_ensembleName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummarySumoDataSource::setEnsembleName( const QString& ensembleName )
{
    m_ensembleName = ensembleName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimSummarySumoDataSource::realizationIds() const
{
    return m_realizationIds();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummarySumoDataSource::setRealizationIds( const std::vector<QString>& realizationIds )
{
    m_realizationIds = realizationIds;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimSummarySumoDataSource::vectorNames() const
{
    return m_vectorNames();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummarySumoDataSource::setVectorNames( const std::vector<QString>& vectorNames )
{
    m_vectorNames = vectorNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummarySumoDataSource::updateName()
{
    if ( !m_customName().isEmpty() )
    {
        setName( m_customName() );
        return;
    }

    auto name = QString( "%1 (%2)" ).arg( ensembleName(), caseName() );

    setName( name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummarySumoDataSource::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder.addCmdFeature( "RicCreateSumoEnsembleFeature" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummarySumoDataSource::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_vectorNames )
    {
        if ( auto attr = dynamic_cast<caf::PdmUiTreeSelectionEditorAttribute*>( attribute ) )
        {
            attr->showCheckBoxes        = false;
            attr->showContextMenu       = false;
            attr->showToggleAllCheckbox = false;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummarySumoDataSource::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto group = uiOrdering.addNewGroup( "General" );

    group->add( nameField() );
    nameField()->uiCapability()->setUiReadOnly( true );

    group->add( &m_caseId );
    m_caseId.uiCapability()->setUiReadOnly( true );

    group->add( &m_caseName );
    m_caseName.uiCapability()->setUiReadOnly( true );

    group->add( &m_ensembleName );
    m_ensembleName.uiCapability()->setUiReadOnly( true );

    group->add( &m_customName );

    auto summaryInfo = uiOrdering.addNewGroup( "Info" );
    summaryInfo->setCollapsedByDefault();
    summaryInfo->add( &m_realizationInfo );
    summaryInfo->add( &m_vectorNames );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummarySumoDataSource::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_customName )
    {
        updateName();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummarySumoDataSource::realizationInfoText() const
{
    std::vector<int> intValues;
    for ( const auto& realizationId : realizationIds() )
    {
        bool ok    = false;
        int  value = realizationId.toInt( &ok );
        if ( ok )
        {
            intValues.push_back( value );
        }
    }

    auto rangeString = RiaStdStringTools::formatRangeSelection( intValues );
    return QString::fromStdString( rangeString );
}
