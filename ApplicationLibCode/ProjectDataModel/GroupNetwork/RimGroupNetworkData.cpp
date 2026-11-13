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
#include "RiaLogging.h"
#include "Summary/RiaSummaryTools.h"

#include "RifGroupNetworkEnsembleStatistics.h"
#include "RifGroupNetworkReader.h"
#include "RifSummaryReaderInterface.h"

#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiFilePathEditor.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QStringList>
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

template <>
void AppEnum<RimGroupNetworkData::SourceType>::setUp()
{
    addItem( RimGroupNetworkData::SourceType::SUMMARY_CASE, "SUMMARY_CASE", "Summary Case" );
    addItem( RimGroupNetworkData::SourceType::SUMMARY_ENSEMBLE, "SUMMARY_ENSEMBLE", "Summary Ensemble" );
    setDefault( RimGroupNetworkData::SourceType::SUMMARY_CASE );
}

template <>
void AppEnum<RimGroupNetworkData::EnsembleValue>::setUp()
{
    addItem( RimGroupNetworkData::EnsembleValue::REALIZATION, "REALIZATION", "Realization" );
    addItem( RimGroupNetworkData::EnsembleValue::MEAN, "MEAN", "Mean" );
    addItem( RimGroupNetworkData::EnsembleValue::P10, "P10", "P10" );
    addItem( RimGroupNetworkData::EnsembleValue::P50, "P50", "P50" );
    addItem( RimGroupNetworkData::EnsembleValue::P90, "P90", "P90" );
    addItem( RimGroupNetworkData::EnsembleValue::MINIMUM, "MINIMUM", "Min" );
    addItem( RimGroupNetworkData::EnsembleValue::MAXIMUM, "MAXIMUM", "Max" );
    setDefault( RimGroupNetworkData::EnsembleValue::MEAN );
}
} // namespace caf

namespace
{
// The deck is located in the folder of the summary header file, with the same base name. If there is no such file, a single deck
// in the folder is used.
QString findDeckFile( const QString& summaryHeaderFile )
{
    if ( summaryHeaderFile.isEmpty() ) return {};

    if ( const QString deck = RiaEclipseFileNameTools( summaryHeaderFile ).findRelatedDataFile(); !deck.isEmpty() ) return deck;

    const QFileInfo headerInfo( summaryHeaderFile );
    const QDir      folder( headerInfo.absolutePath() );

    const QFileInfo sameBaseName( folder.filePath( headerInfo.completeBaseName() + ".DATA" ) );
    if ( sameBaseName.exists() ) return sameBaseName.absoluteFilePath();

    const auto decks = folder.entryInfoList( { "*.DATA" }, QDir::Files );
    return decks.size() == 1 ? decks.front().absoluteFilePath() : QString();
}

QString findDeckFile( const RimSummaryCase* summaryCase )
{
    return summaryCase ? findDeckFile( summaryCase->summaryHeaderFilename() ) : QString();
}

RimSummaryCase* firstRealization( const RimSummaryEnsemble* ensemble )
{
    if ( !ensemble ) return nullptr;
    const auto cases = ensemble->allSummaryCases();
    return cases.empty() ? nullptr : cases.front();
}

// The deck of the first realization that has a deck
QString findDeckFile( const RimSummaryEnsemble* ensemble )
{
    if ( !ensemble ) return {};
    for ( const auto* realization : ensemble->allSummaryCases() )
    {
        if ( const QString deck = findDeckFile( realization ); !deck.isEmpty() ) return deck;
    }
    return {};
}

RifGroupNetworkEnsembleStatistics::Statistic toStatistic( RimGroupNetworkData::EnsembleValue value )
{
    using Statistic = RifGroupNetworkEnsembleStatistics::Statistic;
    switch ( value )
    {
        case RimGroupNetworkData::EnsembleValue::P10:
            return Statistic::P10;
        case RimGroupNetworkData::EnsembleValue::P50:
            return Statistic::P50;
        case RimGroupNetworkData::EnsembleValue::P90:
            return Statistic::P90;
        case RimGroupNetworkData::EnsembleValue::MINIMUM:
            return Statistic::MINIMUM;
        case RimGroupNetworkData::EnsembleValue::MAXIMUM:
            return Statistic::MAXIMUM;
        default:
            return Statistic::MEAN;
    }
}

struct RealizationDeck
{
    QString name;
    QString summaryHeaderFile;
};

// Compares the networks of the realizations with the reference network. Runs in a worker thread, so only copies of the data are used.
void checkTopology( RigGroupNetwork                          reference,
                    QString                                  referenceDeckFile,
                    std::vector<RealizationDeck>             realizations,
                    std::shared_ptr<const std::atomic<bool>> cancelled )
{
    QStringList differing;
    QStringList unreadable;
    for ( const auto& realization : realizations )
    {
        if ( *cancelled ) return;

        const QString deckFile = findDeckFile( realization.summaryHeaderFile );
        if ( deckFile.isEmpty() || deckFile == referenceDeckFile ) continue;

        const auto network = RifGroupNetworkReader::readFromFile( deckFile, false );
        if ( !network )
            unreadable.push_back( realization.name );
        else if ( !network->hasSameTopology( reference ) )
            differing.push_back( realization.name );
    }

    const auto nameList = []( const QStringList& names )
    {
        constexpr int maxNames = 10;
        QString       text     = names.mid( 0, maxNames ).join( ", " );
        if ( names.size() > maxNames ) text += QString( " and %1 more" ).arg( names.size() - maxNames );
        return text;
    };

    if ( !differing.empty() )
    {
        RiaLogging::warning( QString( "Production network: The network of %1 realization(s) differs from the network in '%2', which is "
                                      "shown: %3" )
                                 .arg( differing.size() )
                                 .arg( referenceDeckFile )
                                 .arg( nameList( differing ) )
                                 .toStdString() );
    }
    if ( !unreadable.empty() )
    {
        RiaLogging::warning( QString( "Production network: Could not read the network of %1 realization(s) to compare with '%2': %3" )
                                 .arg( unreadable.size() )
                                 .arg( referenceDeckFile )
                                 .arg( nameList( unreadable ) )
                                 .toStdString() );
    }
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
    CAF_PDM_InitFieldNoDefault( &m_sourceType, "SourceType", "Source" );
    CAF_PDM_InitFieldNoDefault( &m_summaryCase, "SummaryCase", "Summary Case" );
    CAF_PDM_InitFieldNoDefault( &m_summaryEnsemble, "SummaryEnsemble", "Summary Ensemble" );
    CAF_PDM_InitFieldNoDefault( &m_deckFile, "DeckFile", "Deck File" );
    m_deckFile.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
    m_deckFile.uiCapability()->setAttribute( caf::PdmUiFilePathEditor::Keys::FILE_SELECTION_FILTER,
                                             QString( "Eclipse DATA files (*.DATA);;All files (*.*)" ) );
    CAF_PDM_InitFieldNoDefault( &m_treeType, "TreeType", "Network" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGroupNetworkData::~RimGroupNetworkData()
{
    stopTopologyCheck();
}

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
    const QString previousDeckFile = defaultDeckFile();
    m_sourceType                   = SourceType::SUMMARY_CASE;
    m_summaryCase                  = summaryCase;
    if ( summaryCase ) m_name = "Network - " + summaryCase->displayCaseName();
    updateDeckFile( previousDeckFile );
    clearCache();
    dataChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryEnsemble* RimGroupNetworkData::summaryEnsemble() const
{
    return m_summaryEnsemble();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkData::setSummaryEnsemble( RimSummaryEnsemble* ensemble )
{
    const QString previousDeckFile = defaultDeckFile();
    m_sourceType                   = SourceType::SUMMARY_ENSEMBLE;
    m_summaryEnsemble              = ensemble;
    if ( ensemble ) m_name = "Network - " + ensemble->name();
    updateDeckFile( previousDeckFile );
    clearCache();
    dataChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGroupNetworkData::isEnsemble() const
{
    return m_sourceType() == SourceType::SUMMARY_ENSEMBLE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RimGroupNetworkData::realizations() const
{
    if ( !isEnsemble() || !m_summaryEnsemble() ) return {};
    return m_summaryEnsemble()->allSummaryCases();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimGroupNetworkData::referenceRealization() const
{
    if ( !isEnsemble() ) return nullptr;
    return firstRealization( m_summaryEnsemble() );
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
const RifGroupNetworkSummaryData* RimGroupNetworkData::summaryData( EnsembleValue value, RimSummaryCase* realization ) const
{
    ensureLoaded();

    if ( !isEnsemble() ) return m_summaryCase() ? m_summaryData.get() : nullptr;
    if ( !m_summaryEnsemble() ) return nullptr;

    if ( value == EnsembleValue::REALIZATION )
    {
        RimSummaryCase* summaryCase = realization ? realization : referenceRealization();
        if ( !summaryCase ) return nullptr;

        // A deleted case can be replaced by a new case at the same address, so the entry is validated by the PDM pointer
        auto& entry = m_realizationData[summaryCase];
        if ( !entry.data || entry.summaryCase.isNull() || entry.summaryCase.p() != summaryCase )
        {
            entry.summaryCase = summaryCase;
            entry.data        = createCaseData( summaryCase );
        }
        return entry.data.get();
    }

    auto& data = m_statisticData[value];
    if ( !data )
    {
        if ( !m_ensembleStatistics )
        {
            caf::PdmPointer<RimSummaryEnsemble> ensemble = m_summaryEnsemble();
            m_ensembleStatistics                         = std::make_shared<RifGroupNetworkEnsembleStatistics>(
                [ensemble]() -> std::vector<RifSummaryReaderInterface*>
                {
                    std::vector<RifSummaryReaderInterface*> readers;
                    if ( ensemble.isNull() ) return readers;

                    for ( auto* summaryCase : ensemble->allSummaryCases() )
                    {
                        readers.push_back( summaryCase ? summaryCase->summaryReader() : nullptr );
                    }
                    return readers;
                } );
        }

        // The reference realization gives the units and the node categories
        caf::PdmPointer<RimSummaryEnsemble> ensemble = m_summaryEnsemble();
        data                                         = std::make_unique<RifGroupNetworkSummaryData>(
            [ensemble]() -> RifSummaryReaderInterface*
            {
                auto* reference = ensemble.isNull() ? nullptr : firstRealization( ensemble.p() );
                return reference ? reference->summaryReader() : nullptr;
            },
            m_treeType(),
            m_ensembleStatistics,
            toStatistic( value ) );
    }
    return data.get();
}

//--------------------------------------------------------------------------------------------------
/// The summary time steps, or the dates of the network definitions if there is no summary data
//--------------------------------------------------------------------------------------------------
const std::vector<std::time_t>& RimGroupNetworkData::timeSteps( EnsembleValue value, RimSummaryCase* realization ) const
{
    ensureLoaded();
    if ( const auto* data = summaryData( value, realization ); data && !data->timeSteps().empty() ) return data->timeSteps();
    return m_networkDates;
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

    const auto&   dates    = m_networkDates;
    const QString deckName = QFileInfo( deckFile() ).fileName();
    if ( dates.empty() ) return QString( "No %1 network in %2" ).arg( m_treeType().uiText(), deckName );

    QString text = QString( "%1: %2 network definition(s)" ).arg( deckName ).arg( dates.size() );
    if ( shownTree )
    {
        text += QString( ", showing the network defined at %1" ).arg( dateText( shownTree->date ) );
    }

    if ( isEnsemble() )
    {
        if ( m_summaryEnsemble() )
            text += QString( ". Ensemble with %1 realization(s)" ).arg( realizations().size() );
        else
            text += ". Select a summary ensemble to show values";
    }
    else if ( !summaryData() )
    {
        text += ". Select a summary case to show values";
    }
    return text;
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
        updateDeckFile( findDeckFile( previousCase ) );
        clearCache();
    }
    else if ( changedField == &m_summaryEnsemble )
    {
        const auto* previousEnsemble = dynamic_cast<RimSummaryEnsemble*>( oldValue.value<caf::PdmPointer<caf::PdmObjectHandle>>().rawPtr() );
        updateDeckFile( findDeckFile( previousEnsemble ) );
        clearCache();
    }
    else if ( changedField == &m_sourceType )
    {
        // The deck file field is used for a summary case only, so the deck of the case is always the default after the switch
        if ( !isEnsemble() ) updateDeckFile( m_deckFile().path() );
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
    else if ( fieldNeedingOptions == &m_summaryEnsemble )
    {
        for ( auto* ensemble : RimProject::current()->summaryEnsembles() )
        {
            if ( ensemble->isEnsemble() )
                options.push_back( caf::PdmOptionItemInfo( ensemble->name(), ensemble, false, ensemble->uiIconProvider() ) );
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkData::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_name );
    uiOrdering.add( &m_sourceType );
    if ( isEnsemble() )
        uiOrdering.add( &m_summaryEnsemble );
    else
        uiOrdering.add( &m_summaryCase );
    if ( !isEnsemble() ) uiOrdering.add( &m_deckFile );
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

        const QString deckFilePath = deckFile();
        if ( !deckFilePath.isEmpty() )
        {
            auto network = RifGroupNetworkReader::readFromFile( deckFilePath );
            if ( network )
            {
                m_network = std::make_unique<RigGroupNetwork>( std::move( *network ) );
                startTopologyCheck();
            }
            else
            {
                m_loadError = network.error();
            }
        }
    }

    if ( m_summaryData ) return;

    m_networkDates = m_network ? m_network->dates( m_treeType() ) : std::vector<std::time_t>();
    m_summaryData  = createCaseData( m_summaryCase() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkData::clearCache()
{
    stopTopologyCheck();
    m_isLoaded = false;
    m_loadError.clear();
    m_network.reset();
    m_ensembleStatistics.reset();
    clearTreeTypeCache();
}

//--------------------------------------------------------------------------------------------------
/// Clears the data that depends on the tree type. The ensemble statistics are cached by summary address, and are kept.
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkData::clearTreeTypeCache()
{
    m_summaryData.reset();
    m_networkDates.clear();
    m_realizationData.clear();
    m_statisticData.clear();
}

//--------------------------------------------------------------------------------------------------
/// Use the deck of the current summary case or ensemble, unless the user has selected a deck that is not the deck of the
/// previous summary case or ensemble
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkData::updateDeckFile( const QString& previousDeckFile )
{
    const QString deckFile = m_deckFile().path();
    if ( !deckFile.isEmpty() && deckFile != previousDeckFile ) return;

    m_deckFile = caf::FilePath( defaultDeckFile() );
}

//--------------------------------------------------------------------------------------------------
/// The deck that is read. The deck file field is used for a summary case only; the deck of an ensemble is found in the
/// realization folders.
//--------------------------------------------------------------------------------------------------
QString RimGroupNetworkData::deckFile() const
{
    return isEnsemble() ? defaultDeckFile() : m_deckFile().path();
}

//--------------------------------------------------------------------------------------------------
/// The deck of the summary case, or of the first realization of the ensemble that has a deck
//--------------------------------------------------------------------------------------------------
QString RimGroupNetworkData::defaultDeckFile() const
{
    return isEnsemble() ? findDeckFile( m_summaryEnsemble() ) : findDeckFile( m_summaryCase() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<RifGroupNetworkSummaryData> RimGroupNetworkData::createCaseData( RimSummaryCase* summaryCase ) const
{
    // The reader is looked up on each access, as the summary case replaces it on reload. The PDM pointer is set to nullptr when
    // the summary case is deleted.
    caf::PdmPointer<RimSummaryCase> casePointer = summaryCase;
    return std::make_unique<RifGroupNetworkSummaryData>( [casePointer]() -> RifSummaryReaderInterface*
                                                         { return casePointer.isNull() ? nullptr : casePointer->summaryReader(); },
                                                         m_treeType() );
}

//--------------------------------------------------------------------------------------------------
/// Compare the networks of the other realizations with the network that is shown, in a worker thread as reading hundreds of
/// decks takes time
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkData::startTopologyCheck() const
{
    stopTopologyCheck();
    if ( !isEnsemble() || !m_network ) return;

    std::vector<RealizationDeck> decks;
    for ( auto* realization : realizations() )
    {
        if ( realization && !realization->summaryHeaderFilename().isEmpty() )
        {
            decks.push_back( { realization->displayCaseName(), realization->summaryHeaderFilename() } );
        }
    }
    if ( decks.size() < 2 ) return;

    m_topologyCheckCancelled = std::make_shared<std::atomic<bool>>( false );
    m_topologyCheck          = std::async( std::launch::async,
                                  checkTopology,
                                  *m_network,
                                  deckFile(),
                                  std::move( decks ),
                                  std::shared_ptr<const std::atomic<bool>>( m_topologyCheckCancelled ) );
}

//--------------------------------------------------------------------------------------------------
/// The check is cancelled between two decks, so this waits for at most one deck to be read
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkData::stopTopologyCheck() const
{
    if ( m_topologyCheckCancelled ) *m_topologyCheckCancelled = true;
    if ( m_topologyCheck.valid() ) m_topologyCheck.wait();
    m_topologyCheck          = {};
    m_topologyCheckCancelled = nullptr;
}
