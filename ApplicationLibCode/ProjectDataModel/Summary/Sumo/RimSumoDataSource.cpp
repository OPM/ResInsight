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

#include "RimSumoDataSource.h"

#include "RiaStdStringTools.h"

#include "RimSummaryEnsembleSumo.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmPointer.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiTextEditor.h"

#include <QCoreApplication>

CAF_PDM_SOURCE_INIT( RimSumoDataSource, "RimSumoDataSource", "RimSummarySumoDataSource" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSumoDataSource::RimSumoDataSource()
{
    CAF_PDM_InitObject( "Sumo Data Source", ":/CloudBlobs.svg" );

    CAF_PDM_InitFieldNoDefault( &m_caseId, "CaseId", "Case Id" );
    CAF_PDM_InitFieldNoDefault( &m_assetName, "AssetName", "Asset Name" );
    CAF_PDM_InitFieldNoDefault( &m_caseName, "CaseName", "Case Name" );
    CAF_PDM_InitFieldNoDefault( &m_ensembleName, "EnsembleName", "Ensemble Name" );
    CAF_PDM_InitFieldNoDefault( &m_customName, "CustomName", "Custom Name" );

    CAF_PDM_InitFieldNoDefault( &m_availableRealizationIds, "RealizationIds", "Available Realization Ids" );
    m_availableRealizationIds.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_realizationFilter,
                       "RealizationFilter",
                       QString(),
                       "Realization Filter",
                       "",
                       "Specify realization numbers. Example: 1-5, 8, 11-20, !4 will include 1, 2, 3, 5, 8, 11-20" );

    CAF_PDM_InitFieldNoDefault( &m_realizationFilterInfo, "RealizationFilterInfo", "Info" );
    m_realizationFilterInfo.registerGetMethod( this, &RimSumoDataSource::realizationFilterInfoText );
    m_realizationFilterInfo.uiCapability()->setUiReadOnly( true );
    m_realizationFilterInfo.uiCapability()->setUiEditorTypeName( caf::PdmUiTextEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_vectorNames, "VectorNames", "Vector Names" );
    m_vectorNames.uiCapability()->setUiHidden( true );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
SumoCaseId RimSumoDataSource::caseId() const
{
    return SumoCaseId( m_caseId() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSumoDataSource::setCaseId( const SumoCaseId& caseId )
{
    m_caseId = caseId.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSumoDataSource::assetName() const
{
    return m_assetName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSumoDataSource::setAssetName( const QString& assetName )
{
    m_assetName = assetName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSumoDataSource::caseName() const
{
    return m_caseName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSumoDataSource::setCaseName( const QString& caseName )
{
    m_caseName = caseName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSumoDataSource::ensembleName() const
{
    return m_ensembleName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSumoDataSource::setEnsembleName( const QString& ensembleName )
{
    m_ensembleName = ensembleName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimSumoDataSource::availableRealizationIds() const
{
    return m_availableRealizationIds();
}

//--------------------------------------------------------------------------------------------------
/// The available realizations are the source of truth. An empty filter selects all of them.
//--------------------------------------------------------------------------------------------------
void RimSumoDataSource::setAvailableRealizationIds( const std::vector<QString>& realizationIds )
{
    m_availableRealizationIds = realizationIds;

    // Show the full range instead of an empty field. Only an empty filter is replaced, keeping a user
    // or project defined filter. Selection is unchanged, as empty and full range both select all.
    if ( m_realizationFilter().trimmed().isEmpty() )
    {
        m_realizationFilter = availableRealizationsRangeText();
    }
}

//--------------------------------------------------------------------------------------------------
/// The subset of available realizations matching the realization filter. An empty filter (or '*')
/// selects all available realizations.
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimSumoDataSource::selectedRealizationIds() const
{
    const auto& available = m_availableRealizationIds();

    auto filter = m_realizationFilter();
    if ( filter.trimmed().isEmpty() || filter.contains( '*' ) )
    {
        return available;
    }

    auto selectedValues = RiaStdStringTools::valuesFromRangeSelection( filter.toStdString() );

    std::vector<QString> result;
    for ( const auto& realizationId : available )
    {
        bool ok    = false;
        int  value = realizationId.toInt( &ok );
        if ( ok && selectedValues.count( value ) > 0 )
        {
            result.push_back( realizationId );
        }
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimSumoDataSource::vectorNames() const
{
    return m_vectorNames();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSumoDataSource::setVectorNames( const std::vector<QString>& vectorNames )
{
    m_vectorNames = vectorNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSumoDataSource::updateName()
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
void RimSumoDataSource::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder.addCmdFeature( "RicCreateSumoEnsembleFeature" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSumoDataSource::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_realizationFilter )
    {
        if ( auto lineEdAttr = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute ) )
        {
            lineEdAttr->placeholderText = "E.g. 0,1,4-10,!6. Use '*' for all.";
        }
    }
    else if ( field == &m_realizationFilterInfo )
    {
        if ( auto* myAttr = dynamic_cast<caf::PdmUiTextEditorAttribute*>( attribute ) )
        {
            myAttr->heightHint = -1;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSumoDataSource::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto group = uiOrdering.addNewGroup( "General" );

    group->add( nameField() );
    nameField()->uiCapability()->setUiReadOnly( true );

    group->add( &m_caseId );
    m_caseId.uiCapability()->setUiReadOnly( true );

    group->add( &m_assetName );
    m_assetName.uiCapability()->setUiReadOnly( true );

    group->add( &m_caseName );
    m_caseName.uiCapability()->setUiReadOnly( true );

    group->add( &m_ensembleName );
    m_ensembleName.uiCapability()->setUiReadOnly( true );

    group->add( &m_customName );

    auto ensembleGroup = uiOrdering.addNewGroup( "Ensemble Selection" );
    ensembleGroup->add( &m_realizationFilter );
    ensembleGroup->add( &m_realizationFilterInfo );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSumoDataSource::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_customName )
    {
        updateName();
    }
    else if ( changedField == &m_realizationFilter )
    {
        onRealizationFilterChanged();
    }
}

//--------------------------------------------------------------------------------------------------
/// Propagate a change in the realization filter to the ensembles created from this data source, so
/// editing the filter updates the realization cases (and the connected plots/views).
//--------------------------------------------------------------------------------------------------
void RimSumoDataSource::onRealizationFilterChanged()
{
    // Update any summary ensembles created from this data source (same behaviour as summary ensembles
    // loaded from disk, see RimSummaryFileSetEnsemble::onFileSetChanged).
    for ( auto ensemble : objectsWithReferringPtrFieldsOfType<RimSummaryEnsembleSumo>() )
    {
        ensemble->onRealizationSelectionChanged();
    }
}

//--------------------------------------------------------------------------------------------------
/// Info text shown next to the realization filter.
//--------------------------------------------------------------------------------------------------
QString RimSumoDataSource::realizationFilterInfoText() const
{
    return "Available realizations: " + availableRealizationsRangeText();
}

//--------------------------------------------------------------------------------------------------
/// Compact range text for the available ensemble realizations, e.g. "0-99".
//--------------------------------------------------------------------------------------------------
QString RimSumoDataSource::availableRealizationsRangeText() const
{
    std::vector<int> intValues;
    for ( const auto& realizationId : m_availableRealizationIds() )
    {
        bool ok    = false;
        int  value = realizationId.toInt( &ok );
        if ( ok ) intValues.push_back( value );
    }

    auto rangeString = RiaStdStringTools::formatRangeSelection( intValues );
    return QString::fromStdString( rangeString );
}
