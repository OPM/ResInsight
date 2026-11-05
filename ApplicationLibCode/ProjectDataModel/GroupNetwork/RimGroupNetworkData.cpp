/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RimGroupNetworkData.h"

#include "RiaEclipseFileNameTools.h"
#include "Summary/RiaSummaryTools.h"

#include "RifGroupNetworkReader.h"
#include "RifSummaryReaderInterface.h"

#include "RimSummaryCase.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiFilePathEditor.h"

#include <QDateTime>
#include <QFileInfo>
#include <QTimeZone>

namespace caf
{
template <>
void AppEnum<RigGroupNetwork::TreeType>::setUp()
{
    addItem( RigGroupNetwork::TreeType::GRUPTREE, "GRUPTREE", "Standard Network (GRUPTREE)" );
    addItem( RigGroupNetwork::TreeType::BRANPROP, "BRANPROP", "Extended Network (BRANPROP)" );
    setDefault( RigGroupNetwork::TreeType::GRUPTREE );
}
} // namespace caf

namespace
{
QString findDeckFile( const RimSummaryCase* summaryCase )
{
    if ( !summaryCase || summaryCase->summaryHeaderFilename().isEmpty() ) return {};
    return RiaEclipseFileNameTools( summaryCase->summaryHeaderFilename() ).findRelatedDataFile();
}
} // namespace

CAF_PDM_SOURCE_INIT( RimGroupNetworkData, "RimGroupNetworkData" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGroupNetworkData::RimGroupNetworkData()
    : dataChanged( this )
{
    CAF_PDM_InitObject( "Production Network", ":/summary/components/images/production-network.svg" );
    setDeletable( true );

    CAF_PDM_InitField( &m_name, "Name", QString( "Production Network" ), "Name" );
    CAF_PDM_InitFieldNoDefault( &m_summaryCase, "SummaryCase", "Summary Case" );
    CAF_PDM_InitFieldNoDefault( &m_deckFile, "DeckFile", "Deck File" );
    m_deckFile.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
    m_deckFile.uiCapability()->setAttribute( caf::PdmUiFilePathEditor::Keys::FILE_SELECTION_FILTER,
                                             QString( "Eclipse DATA files (*.DATA);;All files (*.*)" ) );
    CAF_PDM_InitFieldNoDefault( &m_treeType, "TreeType", "Network" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGroupNetworkData::~RimGroupNetworkData() = default;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGroupNetworkData::name() const
{
    return m_name();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimGroupNetworkData::summaryCase() const
{
    return m_summaryCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkData::setSummaryCase( RimSummaryCase* summaryCase )
{
    const RimSummaryCase* previousCase = m_summaryCase();
    m_summaryCase                      = summaryCase;
    if ( summaryCase ) m_name = "Network - " + summaryCase->displayCaseName();
    updateDeckFile( previousCase );
    clearCache();
    dataChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigGroupNetwork* RimGroupNetworkData::network() const
{
    ensureLoaded();
    return m_network.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RifGroupNetworkSummaryData* RimGroupNetworkData::summaryData() const
{
    ensureLoaded();
    return m_summaryCase() ? m_summaryData.get() : nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGroupNetwork::TreeType RimGroupNetworkData::treeType() const
{
    return m_treeType();
}

//--------------------------------------------------------------------------------------------------
/// The status of the data, given the network definition that is shown by a plot
//--------------------------------------------------------------------------------------------------
QString RimGroupNetworkData::statusText( const RigGroupNetwork::TreeRevision* shownTree ) const
{
    ensureLoaded();
    if ( !m_loadError.isEmpty() ) return m_loadError;
    if ( !m_network ) return "No deck file";

    const auto& dates = m_networkDates;
    if ( dates.empty() ) return QString( "No %1 network in %2" ).arg( m_treeType().uiText(), QFileInfo( m_deckFile().path() ).fileName() );

    QString text = QString( "%1: %2 network definition(s)" ).arg( QFileInfo( m_deckFile().path() ).fileName() ).arg( dates.size() );
    if ( shownTree )
    {
        text += QString( ", showing the network defined at %1" ).arg( dateText( shownTree->date ) );
    }
    if ( !summaryData() ) text += ". Select a summary case to show values";
    return text;
}

//--------------------------------------------------------------------------------------------------
/// The summary time steps, or the dates of the network definitions if there is no summary data
//--------------------------------------------------------------------------------------------------
const std::vector<std::time_t>& RimGroupNetworkData::timeSteps() const
{
    ensureLoaded();
    if ( const auto* data = summaryData(); data && !data->timeSteps().empty() ) return data->timeSteps();
    return m_networkDates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigGroupNetwork::TreeRevision* RimGroupNetworkData::treeAtDate( std::time_t date ) const
{
    ensureLoaded();
    if ( !m_network ) return nullptr;
    return m_network->treeAtDate( m_treeType(), date );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkData::reload()
{
    clearCache();
    dataChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGroupNetworkData::dateText( std::time_t date )
{
    return QDateTime::fromSecsSinceEpoch( date, QTimeZone::utc() ).toString( "yyyy-MM-dd" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimGroupNetworkData::userDescriptionField()
{
    return &m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkData::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_summaryCase )
    {
        const auto* previousCase = dynamic_cast<RimSummaryCase*>( oldValue.value<caf::PdmPointer<caf::PdmObjectHandle>>().rawPtr() );
        updateDeckFile( previousCase );
        clearCache();
    }
    else if ( changedField == &m_deckFile )
    {
        clearCache();
    }
    else if ( changedField == &m_treeType )
    {
        // The network holds the trees of both types, so only the summary binding depends on the tree type
        clearTreeTypeCache();
    }
    else if ( changedField == &m_name )
    {
        return;
    }

    dataChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimGroupNetworkData::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &m_summaryCase )
    {
        options = RiaSummaryTools::optionsForAllSummaryCases();
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkData::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_name );
    uiOrdering.add( &m_summaryCase );
    uiOrdering.add( &m_deckFile );
    uiOrdering.add( &m_treeType );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkData::initAfterRead()
{
    clearCache();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkData::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicNewGroupNetworkPlotFeature";
    menuBuilder << "RicNewGroupNetworkDataFeature";
    menuBuilder.addSeparator();
    menuBuilder << "RicDeleteItemFeature";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkData::ensureLoaded() const
{
    if ( !m_isLoaded )
    {
        m_isLoaded = true;

        const QString deckFile = m_deckFile().path();
        if ( !deckFile.isEmpty() )
        {
            auto network = RifGroupNetworkReader::readFromFile( deckFile );
            if ( network )
                m_network = std::make_unique<RigGroupNetwork>( std::move( *network ) );
            else
                m_loadError = network.error();
        }
    }

    if ( m_summaryData ) return;

    m_networkDates = m_network ? m_network->dates( m_treeType() ) : std::vector<std::time_t>();

    // The reader is looked up on each access, as the summary case replaces it on reload. The pointer field is set to
    // nullptr when the summary case is deleted. The summary data is owned by this object, so capturing this is safe.
    m_summaryData = std::make_unique<RifGroupNetworkSummaryData>( [this]() -> RifSummaryReaderInterface*
                                                                  { return m_summaryCase() ? m_summaryCase()->summaryReader() : nullptr; },
                                                                  m_treeType() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkData::clearCache()
{
    m_isLoaded = false;
    m_loadError.clear();
    m_network.reset();
    clearTreeTypeCache();
}

//--------------------------------------------------------------------------------------------------
/// Clears the data that depends on the tree type
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkData::clearTreeTypeCache()
{
    m_summaryData.reset();
    m_networkDates.clear();
}

//--------------------------------------------------------------------------------------------------
/// Use the deck of the current summary case, unless the user has selected a deck that is not the deck of the
/// previous summary case
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkData::updateDeckFile( const RimSummaryCase* previousCase )
{
    const QString deckFile = m_deckFile().path();
    if ( !deckFile.isEmpty() && deckFile != findDeckFile( previousCase ) ) return;

    m_deckFile = caf::FilePath( findDeckFile( m_summaryCase() ) );
}
