/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RimSummaryCase.h"

#include "RiaEnsembleNameTools.h"

#include "RicfCommandObject.h"
#include "RifSummaryReaderInterface.h"

#include "RimProject.h"
#include "RimSummaryAddressCollection.h"
#include "RimSummaryEnsemble.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiTreeOrdering.h"

CAF_PDM_ABSTRACT_SOURCE_INIT( RimSummaryCase, "SummaryCase" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase::RimSummaryCase()
    : nameChanged( this )
{
    CAF_PDM_InitScriptableObject( "Summary Case", ":/SummaryCase.svg", "", "The Base Class for all Summary Cases" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_displayName, "ShortName", "Display Name" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_displayNameOption, "NameSetting", "Name Setting" );

    CAF_PDM_InitScriptableField( &m_showSubNodesInTree, "ShowSubNodesInTree", true, "Show Summary Data Sub-Tree" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_showSubNodesInTree );

    CAF_PDM_InitScriptableField( &m_useAutoShortName_OBSOLETE, "AutoShortyName", false, "Use Auto Display Name" );
    m_useAutoShortName_OBSOLETE.xmlCapability()->setIOWritable( false );
    m_useAutoShortName_OBSOLETE.uiCapability()->setUiHidden( true );

    CAF_PDM_InitScriptableFieldNoDefault( &m_summaryHeaderFilename, "SummaryHeaderFilename", "Summary Header File" );
    m_summaryHeaderFilename.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitScriptableField( &m_caseId, "Id", -1, "Case ID" );
    m_caseId.registerKeywordAlias( "CaseId" );
    m_caseId.uiCapability()->setUiReadOnly( true );
    m_caseId.capability<caf::PdmAbstractFieldScriptingCapability>()->setIOWriteable( false );

    CAF_PDM_InitFieldNoDefault( &m_dataVectorFolders, "DataVectorFolders", "Data Folders" );
    m_dataVectorFolders = new RimSummaryAddressCollection();
    m_dataVectorFolders.uiCapability()->setUiHidden( true );
    m_dataVectorFolders.xmlCapability()->disableIO();

    m_isObservedData = false;
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCase::summaryHeaderFilename() const
{
    return m_summaryHeaderFilename().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::setSummaryHeaderFileName( const QString& fileName )
{
    m_summaryHeaderFilename = fileName;

    updateAutoShortName();
    updateTreeItemName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryCase::isObservedData() const
{
    return m_isObservedData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryCase::showVectorItemsInProjectTree() const
{
    return m_showSubNodesInTree();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::setShowVectorItemsInProjectTree( bool enable )
{
    m_showSubNodesInTree = enable;
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::setCaseRealizationParameters( const std::shared_ptr<RigCaseRealizationParameters>& crlParameters )
{
    m_crlParameters = crlParameters;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<RigCaseRealizationParameters> RimSummaryCase::caseRealizationParameters() const
{
    return m_crlParameters;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryCase::hasCaseRealizationParameters() const
{
    return m_crlParameters != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryEnsemble* RimSummaryCase::ensemble() const
{
    RimSummaryEnsemble* e = firstAncestorOrThisOfType<RimSummaryEnsemble>();
    return e && e->isEnsemble() ? e : nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::copyFrom( const RimSummaryCase& rhs )
{
    m_displayName               = rhs.m_displayName;
    m_useAutoShortName_OBSOLETE = rhs.m_useAutoShortName_OBSOLETE;
    m_summaryHeaderFilename     = rhs.m_summaryHeaderFilename;
    m_isObservedData            = rhs.m_isObservedData;

    updateTreeItemName();
}

//--------------------------------------------------------------------------------------------------
/// Sorting operator for sets and maps. Sorts by summary case short name.
//--------------------------------------------------------------------------------------------------
bool RimSummaryCase::operator<( const RimSummaryCase& rhs ) const
{
    return caseName() < rhs.caseName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_displayNameOption )
    {
        updateAutoShortName();
        nameChanged.send();
    }
    else if ( changedField == &m_displayName )
    {
        m_displayNameOption = RimCaseDisplayNameTools::DisplayName::CUSTOM;
        updateTreeItemName();
        nameChanged.send();
    }
    else if ( changedField == &m_showSubNodesInTree )
    {
        updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderRftInterface* RimSummaryCase::rftReader()
{
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCase::errorMessagesFromReader()
{
    return QString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::buildChildNodes()
{
    m_dataVectorFolders->deleteChildren();

    RifSummaryReaderInterface* reader = summaryReader();
    if ( !reader ) return;

    auto addresses = reader->allResultAddresses();
    m_dataVectorFolders->updateFolderStructure( addresses, m_caseId );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSummaryCase::serialNumber()
{
    auto reader = summaryReader();
    return reader ? reader->serialNumber() : -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    appendMenuItems( menuBuilder, true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder, bool showCurveCalculator ) const
{
    menuBuilder << "RicShowDataSourcesForRealization";
    menuBuilder.addSeparator();

    menuBuilder.subMenuStart( "Import" );
    menuBuilder << "RicImportSummaryCaseFeature";
    menuBuilder << "RicImportSummaryCasesFeature";
    menuBuilder << "RicImportSummaryGroupFeature";
    menuBuilder << "RicImportEnsembleFeature";
    menuBuilder.subMenuEnd();
    menuBuilder << "RicCreateSummaryEnsembleFeature";
    menuBuilder.addSeparator();
    menuBuilder << "RicNewSummaryMultiPlotFeature";
    menuBuilder << "RicOpenSummaryPlotEditorFeature";
    menuBuilder << "RicAppendSummaryCurvesForSummaryCasesFeature";
    menuBuilder << "RicAppendSummaryPlotsForSummaryCasesFeature";
    menuBuilder.addSeparator();
    menuBuilder << "RicImportGridModelFromSummaryCaseFeature";

    if ( showCurveCalculator )
    {
        menuBuilder << "RicShowSummaryCurveCalculatorFeature";
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSummaryCase::userDescriptionField()
{
    return &m_displayName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_displayName );
    uiOrdering.add( &m_displayNameOption );
    uiOrdering.add( &m_summaryHeaderFilename );
    uiOrdering.add( &m_caseId );

    if ( ensemble() ) uiOrdering.add( &m_showSubNodesInTree );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    if ( !ensemble() || m_showSubNodesInTree() )
    {
        if ( m_dataVectorFolders->isEmpty() ) buildChildNodes();
        m_dataVectorFolders->updateUiTreeOrdering( uiTreeOrdering );
    }

    uiTreeOrdering.skipRemainingChildren( true );

    updateTreeItemName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::updateTreeItemName()
{
    setUiName( displayCaseName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCase::displayCaseName() const
{
    return m_displayName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCase::nativeCaseName() const
{
    return caseName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCaseDisplayNameTools::DisplayName RimSummaryCase::displayNameType() const
{
    return m_displayNameOption();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::AppEnum<RiaDefines::EclipseUnitSystem> RimSummaryCase::unitsSystem()
{
    RifSummaryReaderInterface* reader = summaryReader();
    if ( reader )
    {
        return reader->unitSystem();
    }
    return RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::setDisplayNameOption( RimCaseDisplayNameTools::DisplayName displayNameOption )
{
    m_displayNameOption = displayNameOption;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::initAfterRead()
{
    if ( m_caseId() == -1 )
    {
        RimProject::current()->assignCaseIdToSummaryCase( this );
    }

    if ( m_useAutoShortName_OBSOLETE )
    {
        m_displayNameOption = RimCaseDisplayNameTools::DisplayName::SHORT_CASE_NAME;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::updateAutoShortName()
{
    if ( m_displayNameOption == RimCaseDisplayNameTools::DisplayName::FULL_CASE_NAME )
    {
        m_displayName = caseName();
    }
    else if ( m_displayNameOption == RimCaseDisplayNameTools::DisplayName::SHORT_CASE_NAME )
    {
        if ( ensemble() )
        {
            m_displayName = RiaEnsembleNameTools::uniqueShortNameForEnsembleCase( this );
        }
        else
        {
            m_displayName = RiaEnsembleNameTools::uniqueShortNameForSummaryCase( this );
        }
    }
    updateTreeItemName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::setCaseId( int caseId )
{
    m_caseId = caseId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSummaryCase::caseId() const
{
    return m_caseId();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::setCustomCaseName( const QString& caseName )
{
    m_displayNameOption = RimCaseDisplayNameTools::DisplayName::CUSTOM;
    m_displayName       = caseName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::refreshMetaData()
{
    buildChildNodes();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::onCalculationUpdated()
{
    // NB! Performance critical method
    if ( !m_showSubNodesInTree ) return;

    if ( m_dataVectorFolders->isEmpty() )
    {
        // Build the child nodes if they are not already built. This function will also create the
        // calculated objects so we can do a early return.
        refreshMetaData();

        return;
    }

    // Delete all calculated address objects
    auto deletedCalculatedObjectCount = m_dataVectorFolders->deleteCalculatedAddresses();
    int  calculatedAddressCount       = 0;

    if ( auto reader = summaryReader() )
    {
        auto addresses = reader->allResultAddresses();

        std::set<RifEclipseSummaryAddress> calculatedAddresses;

        for ( const auto& adr : addresses )
        {
            if ( adr.isCalculated() )
            {
                calculatedAddresses.insert( adr );
            }
        }

        calculatedAddressCount = static_cast<int>( calculatedAddresses.size() );

        m_dataVectorFolders->updateFolderStructure( calculatedAddresses, m_caseId );
    }

    if ( deletedCalculatedObjectCount > 0 || calculatedAddressCount > 0 )
    {
        updateConnectedEditors();
    }
}
